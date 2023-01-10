# Scene File Format
This is the tag-based file format that is used by the parsing interface to describe scenes.

DISCLAIMER: The implemented parser is very basic, so the scene files must follow a very strict format.

# Tags

## Scene
```<Scene options={}></Scene>```

options is a key-value pairing of options specific to the Scene interface.

Available options are 

```bias:       [float]```

```exposure:   [float]```

```maxBounces: [int]```

```numRays:    [int]```

```fisheye:    [int]```

```focus:      [float]```

```lens:       [float]```

## Camera
```<Camera options={}/>```

options is a key-value pairing of options specific to the Camera interface.

Available options are 

```eye:     [(float, float, float)]```

```forward: [(float, float, float)]```

```up:      [(float, float, float)]```

## Lights
```<Light type="" path="" options={}></Light>```

type is one of distant, point, or environment.

path is optional and is only available for type="environment". It is a file path to the .png luminance map to load.

options is a key-value pairing of options specific to the type.

DistantLight options

```direction: [(float, float, float)]```

```color:     [(float, float, float)]```

PointLight options

```center: [(float, float, float)]```

```color:  [(float, float, float)]```

EnvironmentLight options

```radius: [float]```

```color:  [(float, float, float)]```

## Shapes
```<Shape type="" options={}></Shape>```

type is one of triangle, sphere, or plane.

options is a key-value pairing of options specific to the Shape interface.

Triangle options

```p1:     [(float, float, float)]```

```p2:     [(float, float, float)]```

```p3:     [(float, float, float)]```

```color:  [(float, float, float)]```

Sphere options

```center: [(float, float, float)]```

```radius: [float]```

```color:  [(float, float, float)]```

Plane options

```normal: [(float, float, float)]```

```D:      [float]```

```color:  [(float, float, float)]```

## Materials
```<Material name="" options={}/>```

name is optional. If provided, it finds the corresponding material with the given name if it exists. Otherwise, it creates a default material. If name isn't provided, options are used to create the material.

options is a key-value pairing of options specific to the Material interface.

Available options are 

```Kd:        [float]```

```Ks:        [float]```

```eta:       [float]```

```Kr:        [float]```

```Kt:        [float]```

```Ka:        [float]```

```roughness: [float]```

```type:      [string]```

## Textures
```<Texture path=""/>```

path is a file path to the png image to be used for a shape.

## Wavefront .obj file
```<Wavefront path="" options={}></Wavefront>```

path is a file path to the .obj file to load.

options is a key-value pairing of options to initialize the object.

Available options are

```center: [(float, float, float)]```

```scale:  [float]```

```color:  [(float, float, float)]```
