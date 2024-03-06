// SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause
import 'dart:async';

import 'package:camera_aurora/camera_viewfinder.dart';
import 'package:camera_platform_interface/camera_platform_interface.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter/foundation.dart';

import 'camera_aurora_platform_interface.dart';

/// A broadcast stream of events from the Aurora OS device orientation.
Stream<String>? get cameraSearchQr {
  if (TargetPlatform.aurora == defaultTargetPlatform) {
    return CameraAurora().onSearchQr;
  }
  return null;
}

class CameraAurora extends CameraPlatform {
  /// Registers this class as the default instance of [CameraPlatform].
  static void registerWith() {
    CameraPlatform.instance = CameraAurora();
  }

  Stream<String> get onSearchQr => CameraAuroraPlatform.instance.onChangeQr();

  @override
  Future<List<CameraDescription>> availableCameras() =>
      CameraAuroraPlatform.instance.availableCameras();

  @override
  Future<void> initializeCamera(
    int cameraId, {
    ImageFormatGroup imageFormatGroup = ImageFormatGroup.unknown,
  }) async {}

  @override
  Future<int> createCamera(
    CameraDescription cameraDescription,
    ResolutionPreset? resolutionPreset, {
    bool enableAudio = false,
  }) async {
    return (await CameraAuroraPlatform.instance
            .createCamera(cameraDescription.name))
        .textureId;
  }

  @override
  Future<void> dispose(int cameraId) async {
    await CameraAuroraPlatform.instance.dispose();
  }

  @override
  Future<XFile> takePicture(int cameraId) =>
      CameraAuroraPlatform.instance.takePicture(cameraId);

  @override
  Stream<CameraInitializedEvent> onCameraInitialized(int cameraId) async* {
    yield CameraInitializedEvent(
      cameraId,
      0,
      0,
      ExposureMode.auto,
      true,
      FocusMode.auto,
      true,
    );
  }

  @override
  Stream<DeviceOrientationChangedEvent> onDeviceOrientationChanged() async* {
    yield const DeviceOrientationChangedEvent(DeviceOrientation.landscapeLeft);
  }

  @override
  Widget buildPreview(int cameraId) {
    if (cameraId != 0) {
      return LayoutBuilder(builder: (
        BuildContext context,
        BoxConstraints constraints,
      ) {
        if (constraints.maxWidth.isNaN || constraints.maxHeight.isNaN) {
          return const SizedBox.shrink();
        }

        CameraAuroraPlatform.instance
            .resizeFrame(constraints.maxWidth, constraints.maxHeight);
        return CameraViewfinder(
          width: constraints.maxWidth,
          height: constraints.maxHeight,
        );
      });
    }
    return const SizedBox.shrink();
  }
}
