## The read dataset layouts

Each `IADataImportersContext` descendant is implemented by one importer class under [src/importers](/src/importers). This subsection describes what every one of them expects to find inside the directory named by `IADataImportersContext::set_import_path()`, and what it makes of it. The interface that drives them is described in the [The dataset importers API](/doc/sections/en_US/4-project-structure/4-9-the-dataset-importers-api.md) subsection.

### What they all have in common

- The recovered annotations end up as `ImageRecordRect` values - `name`, `x`, `y`, `width` and `height` - always in the image own pixel coordinates, which is what the internal project format stores. The image dimensions land in the `ImageRecord` `iwidth` and `iheight` fields.
- An image path is split the way the internal format keeps it: the directory holding the file becomes `ImageRecord::abs_dir_path` and the file name below it `ImageRecord::path`, so that `ImageRecord::get_full_path()` yields the path the dataset named.
- The library decodes no image format itself. Nothing is copied, moved or rewritten inside the import directory: the produced records point at the image files where they already lie. An import is a read only pass over the dataset.
- What can not be read is logged and skipped, the run itself carries on. A malformed line, an image file the dataset names but does not hold, a picture that can not be measured - each costs its own entry and nothing else.
- The records are handed to the database through `IAnnotationsDB::add_images_db()`, which skips an image the database already holds. Importing one and the same dataset twice adds its images once.
- Five of the nine layouts have to know how large each image is, and ask for it through the `IImageSizeFacility` of the context or the one a build with OpenCV ships: the YOLO v4 one and the three Ultralytics YOLO ones store their boxes normalised, and the PyTorch Vision one has the crop itself for an annotation. An import of such a layout with no reader at all fails at once, and a picture it can not measure is skipped whole.
- The four remaining ones are complete without a measurement. The COCO and the Pascal VOC descriptors carry the size of every image they name, so a reader is only ever asked for the one a foreign tool left it out for; the plain text and the Create ML layouts carry no size anywhere, and their records take the image dimensions from a reader when there is one and stay at zero when there is not. Their rectangles are whole either way, since both layouts hold them in the very pixels the internal format wants them in.

The examples below all describe the very same two record result, which is what the sibling [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) library was given when it wrote those datasets:

| Image | Size | Rectangles |
| --- | --- | --- |
| `/home/user/images/street.png` | 640 x 400 | `dog` (50, 20, 100, 40), `dog` (300, 25, 90, 45) |
| `/home/user/images/park.jpg` | 640 x 480 | `cat` (200, 130, 48, 52), `dog` (12, 8, 64, 64) |

### PlainTxtImportContext

The simplest layout: one plain text file per annotation name, lying straight in the import directory and named `<annotation-name>.txt`.

```
import_path/
|-- cat.txt
`-- dog.txt
```

Every `*.txt` file of the directory is read, and its stem becomes the annotation name of every rectangle recovered out of it. Nothing else is looked at: a file of another extension, and a sub-directory whatever it is called, are both left alone. The files are read in the sorted order of their names, so one and the same directory imports one and the same way twice.

A line carries an image, the number of the rectangles of that name drawn over it and exactly that many coordinate quadruples:

```
<image full path> <rect count> <x> <y> <width> <height> [<x> <y> <width> <height> ...]
```

So a `dog.txt` of

```
/home/user/images/street.png 2 50 20 100 40 300 25 90 45
/home/user/images/park.jpg 1 12 8 64 64
```

and a `cat.txt` of

```
/home/user/images/park.jpg 1 200 130 48 52
```

give back the two records of the table above: `park.jpg` is named by both files and collects the rectangles of both, in the order the files were read.

The image path is written unquoted and the file names no separator, so **the count is what tells the path from the rectangles behind it**: the reader takes the first token which is a whole number with exactly four numbers per rectangle following it, and everything before that token is the path. A path holding a space therefore comes back intact. A line where no such token is found at all - and one promising more or fewer rectangles than it wrote - is dropped whole and logged.

The layout carries no image size, so `iwidth` and `iheight` stay at zero unless an `IImageSizeFacility` is there, in which case every recovered image is measured with it. The rectangles are complete either way, since this layout stores them in the pixels the internal format wants them in.

### Yolo4ImportContext

The darknet training directory of the YOLO v4 detector:

```
import_path/
|-- backup/
|-- cfg/
|   `-- yolov4-obj.cfg
`-- data/
    |-- obj.data
    |-- obj.names
    |-- park.jpg
    |-- park.txt
    |-- street.png
    |-- street.txt
    |-- train.txt
    `-- val.txt
```

`data/obj.data` is read first when it is there, both at that path and at the `obj.data` beside the import directory itself, since the darknet examples keep it in either place. Its `names` and `train` entries then decide which files the class list and the image list are taken from, so a dataset laid out by some other tool is followed rather than assumed:

```
classes = 2
train = data/train.txt
valid = data/val.txt
names = data/obj.names
backup = backup/
```

A path written there is taken relative to the import directory, unless it is absolute. Without the descriptor the conventional `data/obj.names`, `data/train.txt` and `data/val.txt` are used.

`data/obj.names` lists the class names, one per line, and **the line number is the class index** every label file writes - `cat` is the class `0` and `dog` is the class `1` here:

```
cat
dog
```

This file is the one the layout can not be read without: an import finding no name in it fails and leaves the database untouched, since the directory is then no YOLO v4 dataset at all.

`data/train.txt` lists the images, one path per line, relative to the import directory (or absolute):

```
data/street.png
data/park.jpg
```

When it names no image the `valid` list is tried, and when that one names none either the `data` directory itself is walked, everything that is not a `.txt`, a `.names` or a `.data` file being an image of it. An image the list names but the disk does not hold is logged and skipped.

Every image is measured, and a picture that stays unmeasured is skipped whole - its boxes are normalised by a size which stayed unknown, so there is nothing to place them by.

The label file of an image is the `<image stem>.txt` beside it. Each of its lines carries a class index and the box centre and size, all four divided by the size of the image:

```
1 0.1953125 0.1 0.15625 0.1
1 0.5390625 0.1125 0.140625 0.1125
```

The division is undone with the measured size: the centre and the extent are multiplied back, the origin is the centre less half of that extent, and each of the four is rounded to the nearest pixel. The two lines above over a 640 x 400 image give the `dog` rectangles (50, 20, 100, 40) and (300, 25, 90, 45) back exactly.

A line naming a class index the names file does not hold, and a line that is not a class index with four numbers behind it, are both dropped and logged. An image with no label file beside it becomes a record with no rectangles: darknet reads such an image as a picture holding none of the classes, and it belongs to the project just as much.

The `cfg/yolov4-obj.cfg` network descriptor and the empty `backup/` hold nothing of the annotations and are not read at all.

### What the three Ultralytics YOLO layouts share

Three of the contexts read the layout every Ultralytics release trains from - the one YOLO v5 introduced and v8, v11 and the ones after them kept. The directory is the same for all three of them, and so is the `data.yaml` descriptor. What the trained task changes is the single label file line of a rectangle, which is the only thing the three sections below differ in.

```
import_path/
|-- data.yaml
|-- images/
|   `-- train/
|       |-- park.jpg
|       `-- street.png
`-- labels/
    `-- train/
        |-- park.txt
        `-- street.txt
```

`data.yaml` is the whole descriptor of the dataset - this layout has no `obj.names` file of the darknet one - and it is looked for under that very name first, then among the other `*.yaml` and `*.yml` files of the import directory, sorted by name. The first of them declaring a class name is the descriptor; a directory where none does is no dataset of this layout, and the import fails leaving the database untouched.

```yaml
# The Ultralytics YOLO dataset descriptor, written by the ImagesAnnotator
# annotations dataset exporters library.
path: '/home/user/dataset'
train: images/train
val: images/train

names:
  0: 'cat'
  1: 'dog'
```

- **`names`** maps a class index onto an annotation name, and that index is what the label files carry. All three spellings of the entry are read: the indexed block above, the `- <name>` list block and the inline `['cat', 'dog']` one, the latter two numbering their names by position. A name written in either quoting style comes back unquoted, so a colon, a hash or a doubled quote inside it stays a part of the name.
- **`train`** and **`val`** name the image directories, each relative to the dataset root or absolute. Both are read, and a directory named by both of them is read once - which is what the exporting side writes, offering the whole set for the validation as well.
- **`path`** is that dataset root. It is followed only where the import path does not resolve the entry itself: a directory which was moved carries a `path` naming where it no longer is, and dropping that line is what the exporting side documents as the way to move it. A descriptor naming neither part falls back to the `images/train` and the `images` directories of the import path.
- Everything else the entry set of an Ultralytics release accepts describes a training run rather than a dataset, and is read over.

Every file of such a directory which is not a `.txt` one is an image of it - the layout names no image extension anywhere - and the label file of an image is that very path with its last `images` element swapped for `labels` and its extension for `.txt`, which is the pairing a training run performs. An image lying under no `images` element at all is paired with the `<image stem>.txt` beside it instead.

Every image is measured, and a picture that stays unmeasured is skipped whole: all three layouts store their geometry divided by the size of the image it was drawn over, so there is nothing to place it by. An image with no label file becomes a record with no rectangles, the way a training run reads it as a picture holding none of the classes.

A line naming a class index the descriptor does not declare, a line carrying a number count the implemented task never writes, and a line that is no class index with numbers behind it at all, are each dropped and logged. What the read numbers are turned into is the two corners of the rectangle, and the two guards of the exporting side are undone with them:

- **A coordinate outside the `0..1` range is cut down to the image.** An Ultralytics release refuses a whole image over such a coordinate, so this only ever reaches a dataset written by something else - and a box reaching over an edge is cut there exactly as the exporting side cuts it.
- A box left with no area inside the image is dropped, and the image and the rest of its rectangles are imported as usual.

### UltralyticsDetectImportContext

The detection dataset. Every line carries the class index and the box:

```
<class index> <centre x> <centre y> <width> <height>
```

which is the very four numbers the darknet layout writes as well. So a `labels/train/street.txt` of

```
1 0.15625 0.1 0.15625 0.1
1 0.539062 0.11875 0.140625 0.1125
```

over a 640 x 400 `street.png` gives the `dog` rectangles (50, 20, 100, 40) and (300, 25, 90, 45) back: the centre and the extent are multiplied by the measured size, the origin is the centre less half of that extent, and each of the four is rounded to the nearest pixel. A line carrying anything other than four numbers behind its class index is dropped.

### UltralyticsObbImportContext

The oriented bounding box dataset. Every line carries the class index and the four corners of the box, clockwise from the top left one:

```
<class index> <x1> <y1> <x2> <y2> <x3> <y3> <x4> <y4>
```

So a `labels/train/street.txt` of

```
1 0.078125 0.05 0.234375 0.05 0.234375 0.15 0.078125 0.15
1 0.46875 0.0625 0.609375 0.0625 0.609375 0.175 0.46875 0.175
```

gives those same two `dog` rectangles back. The annotations database knows axis aligned rectangles only, so **what comes back is the upright rectangle holding the four corners** - which for a box written by the sibling exporters library, whose rotation angle is always zero, is the very rectangle it was drawn as. A box some other tool wrote with a real angle comes back as the upright one holding it, which is wider than the object it marks. A line carrying anything other than eight numbers behind its class index is dropped.

### UltralyticsSegmentImportContext

The instance segmentation dataset. Every line carries the class index and the points of the polygon which outlines the object, of any three or more of them:

```
<class index> <x1> <y1> ... <xn> <yn>
```

**What comes back is the rectangle holding that polygon.** The mask of a rectangle annotation is the rectangle outline itself, which makes these label files identical to the oriented bounding box ones above, and a dataset written by the sibling exporters library therefore round trips through this layout exactly. A polygon of a real shape - one drawn in a tool which draws them - comes back as the box enclosing it, since the annotations database holds no mask.

A line carrying fewer than six numbers behind its class index, or an odd count of them, is dropped: neither is a ring of `x y` pairs of three points or more.

### CocoImportContext

The COCO object detection dataset: a directory of pictures and the single JSON descriptor over them.

```
import_path/
|-- annotations/
|   `-- instances_default.json
`-- images/
    |-- park.jpg
    `-- street.png
```

The descriptor is looked for in the order below, and the first file which is a JSON object holding an `images` array is the one - that array is what tells such a descriptor from any other JSON file lying beside it:

1. `annotations/instances_default.json`, the name the sibling exporters library writes;
2. the other `*.json` files of `annotations/`, sorted by name, which is where the public releases of the format keep theirs;
3. the `*.json` files of the import directory itself, sorted by name, which is where the tools handing out a single flat directory leave it.

The `file_name` of an image is taken relative to the first of these directories which is there: the `images/` of the exported layout, the one named after the descriptor itself - the `train2017/` beside an `instances_train2017.json` of the public releases - or the import directory, which is where a flat directory holds its pictures.

```json
{
"info": {"description": "The ImagesAnnotator annotations dataset", "version": "0.12.0"},
"licenses": [],
"images": [
  {"id": 1, "file_name": "street.png", "width": 640, "height": 400},
  {"id": 2, "file_name": "park.jpg", "width": 640, "height": 480}
],
"annotations": [
  {"id": 1, "image_id": 1, "category_id": 2, "bbox": [50, 20, 100, 40], "area": 4000, "iscrowd": 0, "segmentation": []},
  {"id": 2, "image_id": 1, "category_id": 2, "bbox": [300, 25, 90, 45], "area": 4050, "iscrowd": 0, "segmentation": []},
  {"id": 3, "image_id": 2, "category_id": 1, "bbox": [200, 130, 48, 52], "area": 2496, "iscrowd": 0, "segmentation": []},
  {"id": 4, "image_id": 2, "category_id": 2, "bbox": [12, 8, 64, 64], "area": 4096, "iscrowd": 0, "segmentation": []}
],
"categories": [
  {"id": 1, "name": "cat", "supercategory": ""},
  {"id": 2, "name": "dog", "supercategory": ""}
]
}
```

That descriptor gives back the two records of the table above, and nothing of a rectangle is computed to do it:

- **`bbox`** is `[x, y, width, height]` of the top left corner, in the image own pixels - the very four fields an `ImageRecordRect` holds. A value written with a fractional part is rounded to the nearest pixel, and a box left with no width or height is dropped.
- **`categories`** names the annotations, and an annotation naming a `category_id` the descriptor does not declare is dropped: an unnamed rectangle is no annotation. The identifiers themselves are read as they stand, so a descriptor numbering its categories from anywhere is followed.
- **`images`** carries the `width` and the `height` of every picture, which is why this layout needs no measurement. An image a foreign tool declared without them is measured when a reader is there, and stays at zero when there is not. An image the directory does not hold becomes a record all the same, so that a project whose pictures were moved is repaired by pointing it at them again rather than by importing it once more.
- **`area`**, **`iscrowd`** and **`segmentation`** are read over: the first is the box multiplied out, and the annotations database holds neither a crowd flag nor a mask.
- An annotation naming an image the descriptor does not declare, and one carrying no four numbers in its `bbox`, are each dropped and logged. An image no annotation names becomes a record with no rectangles.

### PascalVocImportContext

The Pascal VOC dataset, in the devkit directory shape: the pictures, one XML descriptor per picture and the image lists naming those.

```
import_path/
|-- Annotations/
|   |-- park.xml
|   `-- street.xml
|-- ImageSets/
|   `-- Main/
|       |-- train.txt
|       `-- val.txt
`-- JPEGImages/
    |-- park.jpg
    `-- street.png
```

Every `*.xml` file of `Annotations/` is read, in the sorted order of the names. A directory carrying no such sub-directory is read as a flat one - its own `*.xml` files are the descriptors then - which is the shape [LabelImg](https://github.com/HumanSignal/labelImg) saves its own work in.

`ImageSets/Main` is not read at all. Its files name a training and a validation part of one and the same set - identical lists, as the exporting side writes them - while a project holds the images themselves and no split of them, so every descriptor of the directory is read whichever list happens to name it.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<annotation>
  <folder>JPEGImages</folder>
  <filename>street.png</filename>
  <source>
    <database>The ImagesAnnotator annotations dataset</database>
  </source>
  <size>
    <width>640</width>
    <height>400</height>
    <depth>3</depth>
  </size>
  <segmented>0</segmented>
  <object>
    <name>dog</name>
    <pose>Unspecified</pose>
    <truncated>0</truncated>
    <difficult>0</difficult>
    <bndbox>
      <xmin>50</xmin>
      <ymin>20</ymin>
      <xmax>150</xmax>
      <ymax>60</ymax>
    </bndbox>
  </object>
</annotation>
```

- **`bndbox`** is the two corner points the rectangle was drawn between, in the image own pixels, and the record holds the origin and the size: `xmin`/`ymin` is that origin and `xmax` less `xmin` its width. A descriptor of the original VOC devkit, whose coordinates count from one instead of from zero, therefore comes back as the very same box moved by a single pixel - both corners carry that one, so the size of it stays. A box left with no width or height is dropped.
- **`filename`** names the picture, and it is looked for in the first of these directories which really holds it: the `JPEGImages/` of the devkit shape, the one the `folder` element names, the directory of the descriptor itself and the import directory. A file none of them holds becomes a record pointing at the first of them all the same.
- **`name`** is the annotation name, read XML unescaped - an `&amp;`, a `&lt;` or a numeric `&#233;` inside it comes back as the symbol it names. An object naming nothing is dropped.
- **`size`** fills the record dimensions in. A descriptor written without that element is measured when a reader is there.
- **`truncated`**, **`difficult`**, **`pose`**, **`segmented`** and **`source`** are read over. The first marks a box the image edge cut down, which the annotations database holds nothing of, and a `difficult` rectangle is imported like every other one: a project is edited and not evaluated.
- A file which is no `annotation` element, and one naming no image, are each skipped whole and logged, while the rest of the directory is imported as usual.

### CreateMLImportContext

The Create ML object detection dataset: the pictures and the one JSON descriptor beside them, in a single flat directory.

```
import_path/
|-- annotations.json
|-- park.jpg
`-- street.png
```

The flatness is the format: this is the `directoryWithImagesAndJsonAnnotation` data source of Apple's `MLObjectDetector`, a directory of images holding exactly one JSON annotation file. That file is looked for under the `annotations.json` name the data source demands first, and then among the other `*.json` files of the directory, sorted by name - which is where an export whose project held an image of that very name left it. The first of them which is an array of elements naming an image file is the descriptor.

```json
[
  {"imagefilename": "street.png", "annotation": [
    {"label": "dog", "coordinates": {"x": 100, "y": 40, "width": 100, "height": 40}},
    {"label": "dog", "coordinates": {"x": 345, "y": 47.5, "width": 90, "height": 45}}
  ]},
  {"imagefilename": "park.jpg", "annotation": [
    {"label": "cat", "coordinates": {"x": 224, "y": 156, "width": 48, "height": 52}},
    {"label": "dog", "coordinates": {"x": 44, "y": 40, "width": 64, "height": 64}}
  ]}
]
```

- **`x` and `y` are the centre of the box**, not its corner, counted in the image own pixels from the top left of the image - the one halving this import undoes. The origin an `ImageRecordRect` holds is that centre less half of the size, and the `47.5` of a box of an odd size lands back on the very pixel it was drawn from.
- **`width` and `height`** are the size of the box in those same pixels, which the record keeps unchanged. A box left with no width or height is dropped.
- **`label`** is the annotation name, read as it stands. An annotation naming nothing is dropped.
- **`imagefilename`** is the file name alone, taken relative to the import directory. An image the directory does not hold becomes a record all the same, the way it does in the COCO layout above.
- **Both spellings of the two keys are read**: the singular `imagefilename` and `annotation` pair Apple documents, and the plural `image` and `annotations` one some converters emit instead.
- The layout carries no image size anywhere, so the record dimensions are filled in from a reader when the consumer supplied one and stay at zero when they are not. An image whose `annotation` array is empty becomes a record with no rectangles.

### PyTorchImportContext

The classification layout the PyTorch Vision `ImageFolder` dataset reads: one directory per annotation name, holding the images cropped down to the rectangles of that name.

```
import_path/
|-- cat/
|   `-- park.png
`-- dog/
    |-- park.png
    |-- street.png
    `-- street-0.png
```

Every immediate sub-directory is a class and its name is the annotation name; a file lying beside them is ignored, as is a directory nested below a class. Both levels are walked in the sorted order of their names.

This layout has lost the pictures the crops were cut out of, so **every crop becomes an image record of its own**, carrying the one rectangle over its whole area:

| Image | Size | Rectangles |
| --- | --- | --- |
| `import_path/cat/park.png` | 48 x 52 | `cat` (0, 0, 48, 52) |
| `import_path/dog/park.png` | 64 x 64 | `dog` (0, 0, 64, 64) |
| `import_path/dog/street.png` | 100 x 40 | `dog` (0, 0, 100, 40) |
| `import_path/dog/street-0.png` | 90 x 45 | `dog` (0, 0, 90, 45) |

That extent is the size of the file itself, which is why this import measures every crop and skips the ones it can not: a rectangle of an unknown extent is no annotation.

This is the one layout of the nine which does not round trip into the records it was written from. The export cut the rectangles out and dropped everything around them, the original images among it, so what comes back is one image per rectangle rather than one image carrying its rectangles. The class of every crop survives in full, which is what that layout is for.
