# camera_aurora

The Aurora implementation of [camera](https://pub.dev/packages/camera).

## Usage
This package is not an _endorsed_ implementation of `camera`.
Therefore, you have to include `camera_aurora` alongside `camera` as dependencies in your `pubspec.yaml` file.

***.desktop**

```desktop
Permissions=Camera
```

***.spec**

```spec
%global __requires_exclude ^lib(yuv|ZXing|jpeg)\\.so.*$

BuildRequires: pkgconfig(streamcamera)
```

**pubspec.yaml**

```yaml
dependencies:
  camera: ^0.10.5+5
  camera_aurora:
    git:
      url: https://gitlab.com/omprussia/flutter/flutter-plugins.git
      ref: camera_aurora-0.0.1
      path: packages/camera/camera_aurora
```
