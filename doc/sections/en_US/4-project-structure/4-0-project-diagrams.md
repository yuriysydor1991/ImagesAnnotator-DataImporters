## Project diagrams

The class diagram of the `lib` component - the installable interface, the implementation hidden behind it and the database of the data drivers dependency it fills:

![lib component](/doc/diagrams/images/lib-class-structure.svg)

Four groups are drawn:

- `ImagesAnnotatorDataDrivers011` - the database the importers fill and the records they build (`IAnnotationsDB`, `ImageRecord`), owned by the dependency;
- `ImagesAnnotatorDataImporters013` - everything installed from [src/lib/facade/public](/src/lib/facade/public): `LibraryFacade`, `ILib`, `LibraryContext` with its nine layout descendants, `IImporter` and the consumer implemented `IImageSizeFacility`;
- the implementation, whose symbols stay inside the shared object: `LibMain`, `LibFactory`, the nine importers sharing the `Folder2DBImporter` base - with the `JsonDescriptor2DBImporter` and the `UltralyticsFolder2DBImporter` halves the layout families of them share on top of it - and the `OpenCVImageSizer` a build which found OpenCV ships;
- the document readers, hidden just as much: `IJsonParser` with `JsonParser` and the `JsonValue` tree behind it, and `IXmlParser` with `XmlParser` and `XmlNode`, which the descriptor layouts are read through.

The image is rendered from the PlantUML source at [doc/diagrams/plantuml/lib-class-structure.puml](/doc/diagrams/plantuml/lib-class-structure.puml). Edit that file and render it again whenever the public interface changes - the checked in image is the rendering of that source and is not produced by the build:

```
# from the project root directory

plantuml -tsvg -o ../images doc/diagrams/plantuml/lib-class-structure.puml
```
