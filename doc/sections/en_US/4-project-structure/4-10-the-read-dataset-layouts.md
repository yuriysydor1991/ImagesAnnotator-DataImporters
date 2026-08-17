## The read dataset layouts

Each `LibraryContext` descendant is implemented by one importer class under [src/importers](/src/importers). This subsection describes what every one of them expects to find inside the directory named by `LibraryContext::set_import_path()`, and what it makes of it. The interface that drives them is described in the [The dataset importers API](/doc/sections/en_US/4-project-structure/4-9-the-dataset-importers-api.md) subsection.

### What they all have in common

- The recovered annotations end up as `ImageRecordRect` values - `name`, `x`, `y`, `width` and `height` - always in the image own pixel coordinates, which is what the internal project format stores. The image dimensions land in the `ImageRecord` `iwidth` and `iheight` fields.
- An image path is split the way the internal format keeps it: the directory holding the file becomes `ImageRecord::abs_dir_path` and the file name below it `ImageRecord::path`, so that `ImageRecord::get_full_path()` yields the path the dataset named.
- The library decodes no image format itself. Nothing is copied, moved or rewritten inside the import directory: the produced records point at the image files where they already lie. An import is a read only pass over the dataset.
- What can not be read is logged and skipped, the run itself carries on. A malformed line, an image file the dataset names but does not hold, a picture that can not be measured - each costs its own entry and nothing else.
- The records are handed to the database through `IAnnotationsDB::add_images_db()`, which skips an image the database already holds. Importing one and the same dataset twice adds its images once.
- Two of the three layouts have to know how large each image is, and ask for it through the `IImageSizeFacility` of the context or the one a build with OpenCV ships. The plain text layout needs no measurement, and only fills the image dimensions in when a reader happens to be there.

The examples below all describe the very same two record result, which is what the sibling [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) library was given when it wrote those datasets:

| Image | Size | Rectangles |
| --- | --- | --- |
| `/home/user/images/street.png` | 640 x 400 | `dog` (50, 20, 100, 40), `dog` (300, 25, 90, 45) |
| `/home/user/images/park.jpg` | 640 x 480 | `cat` (200, 130, 48, 52), `dog` (12, 8, 64, 64) |

### PlainTxtImportLibraryContext

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

### Yolo4ImportLibraryContext

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

### PyTorchImportLibraryContext

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

This is the one layout of the three which does not round trip into the records it was written from. The export cut the rectangles out and dropped everything around them, the original images among it, so what comes back is one image per rectangle rather than one image carrying its rectangles. The class of every crop survives in full, which is what that layout is for.
