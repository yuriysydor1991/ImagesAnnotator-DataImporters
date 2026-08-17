## Project diagrams

The class diagram of the `lib` component - the installable interface, the implementation hidden behind it and the database of the data drivers dependency it fills:

![lib component](/doc/diagrams/images/lib-class-structure.svg)

Three groups are drawn:

- `ImagesAnnotatorDataDrivers011` - the database the importers fill and the records they build (`IAnnotationsDB`, `ImageRecord`), owned by the dependency;
- `ImagesAnnotatorDataImporters011` - everything installed from [src/lib/facade/public](/src/lib/facade/public): `LibraryFacade`, `ILib`, `LibraryContext` with its three layout descendants, `IImporter` and the consumer implemented `IImageSizeFacility`;
- the implementation, whose symbols stay inside the shared object: `LibMain`, `LibFactory`, the importers sharing the `Folder2DBImporter` base, and the `OpenCVImageSizer` a build which found OpenCV ships.

The image is rendered from the PlantUML source at [doc/diagrams/plantuml/lib-class-structure.puml](/doc/diagrams/plantuml/lib-class-structure.puml). Edit that file and render it again whenever the public interface changes:

```
# from the project root directory

plantuml -tsvg -o ../images doc/diagrams/plantuml/lib-class-structure.puml
```
