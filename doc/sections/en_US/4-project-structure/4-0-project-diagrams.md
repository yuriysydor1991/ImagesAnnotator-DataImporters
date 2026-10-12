## Project diagrams

The class diagram of the `lib` component - the installable interface, the implementation hidden behind it and the database of the data drivers dependency it fills:

![lib component](/doc/diagrams/images/lib-class-structure.svg)

Five groups are drawn:

- `ImagesAnnotatorDataDrivers013` - the database the importers fill and the records they build (`IAnnotationsDB`, `ImageRecord`), owned by the dependency;
- `ImagesAnnotatorDataImporters015` - everything installed from [src/lib/facade/public](/src/lib/facade/public): `IADataImportersFacade`, `IADataImportersLib`, `IADataImportersContext` with its thirteen layout descendants, `IImporter` and the consumer implemented `IImageSizeFacility`;
- the implementation, whose symbols stay inside the shared object: `LibMain`, `LibFactory`, the thirteen importers sharing the `Folder2DBImporter` base - with the `JsonDescriptor2DBImporter` and the `UltralyticsFolder2DBImporter` halves the layout families of them share on top of it, and the two darknetxx training directory importers extending the YOLO v4 one beside the two training project ones, all four reading their dxxwjz files through the `DxxwjzFileReader` - and the `OpenCVImageSizer` a build which found OpenCV ships;
- the document readers, hidden just as much: `IJsonParser` with `JsonParser` and the `JsonValue` tree behind it, and `IXmlParser` with `XmlParser` and `XmlNode`, which the descriptor layouts are read through, and the `JsonMembersPicker` which reads the darknetxx documents a piece at a time;
- [src/zlib](/src/zlib) - the `ZlibController` of the cpp-app-template the `DxxwjzFileReader` decompresses the gzip dxxwjz files with, in the `iadi0zlibi` namespace of this library.

The image is drawn by hand after the PlantUML source at [doc/diagrams/plantuml/lib-class-structure.puml](/doc/diagrams/plantuml/lib-class-structure.puml), which stays the complete description of the component: a fan of thirteen layouts laid out automatically comes out several thousand pixels wide, so the drawing condenses the edges and the methods every layout repeats into one each. Edit the source whenever the public interface changes and bring the drawing along with it - neither is produced by the build. Rendering the source into a directory of its own shows what to compare the drawing against, without overwriting it:

```
# from the project root directory

plantuml -tsvg -o "$(pwd)/build/diagrams" doc/diagrams/plantuml/lib-class-structure.puml
```
