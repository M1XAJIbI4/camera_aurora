// SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause
import 'dart:convert';

import 'package:camera_platform_interface/camera_platform_interface.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'camera_aurora_platform_interface.dart';
import 'camera_data.dart';

enum CameraAuroraMethods {
  availableCameras,
  createCamera,
  resizeFrame,
  dispose,
  startCapture,
  stopCapture,
  takePicture,
}

enum CameraAuroraEvents {
  cameraAuroraStateChanged,
  cameraAuroraQrChanged,
}

/// An implementation of [CameraAuroraPlatform] that uses method channels.
class MethodChannelCameraAurora extends CameraAuroraPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodsChannel = const MethodChannel('camera_aurora');

  @override
  Stream<CameraState> onChangeState() async* {
    await for (final data
        in EventChannel(CameraAuroraEvents.cameraAuroraStateChanged.name)
            .receiveBroadcastStream()) {
      yield CameraState.fromJson(data);
    }
  }

  @override
  Stream<String> onChangeQr() async* {
    await for (final data
        in EventChannel(CameraAuroraEvents.cameraAuroraQrChanged.name)
            .receiveBroadcastStream()) {
      yield data.toString();
    }
  }

  @override
  Future<void> resizeFrame(double width, double height) async {
    await methodsChannel
        .invokeMethod<Object?>(CameraAuroraMethods.resizeFrame.name, {
      'width': width.round(),
      'height': height.round(),
    });
  }

  @override
  Future<List<CameraDescription>> availableCameras() async {
    final List<CameraDescription> result = [];

    final cameras = await methodsChannel.invokeMethod<List<dynamic>?>(
            CameraAuroraMethods.availableCameras.name) ??
        [];

    for (final camera in cameras) {
      final data = camera['id'].split(':');
      var direction = CameraLensDirection.external;

      if (data[1].toString().contains('rear')) {
        direction = CameraLensDirection.back;
      } else if (data[1].toString().contains('front')) {
        direction = CameraLensDirection.front;
      }

      result.add(CameraDescription(
        name: camera['id'],
        lensDirection: direction,
        sensorOrientation: camera['mountAngle'],
      ));
    }

    return result;
  }

  @override
  Future<CameraState> createCamera(String cameraName) async {
    final data = await methodsChannel
        .invokeMethod<Map<dynamic, dynamic>?>('createCamera', {
      'cameraName': cameraName,
    });
    return CameraState.fromJson(data ?? {});
  }

  @override
  Future<void> startCapture(double width, double height) async {
    await methodsChannel
        .invokeMethod<Object?>(CameraAuroraMethods.startCapture.name, {
      'width': width.round(),
      'height': height.round(),
    });
  }

  @override
  Future<void> stopCapture() async {
    await methodsChannel
        .invokeMethod<Object?>(CameraAuroraMethods.stopCapture.name, {});
  }

  @override
  Future<void> dispose() async {
    await methodsChannel
        .invokeMethod<Object?>(CameraAuroraMethods.dispose.name);
  }

  @override
  Future<XFile> takePicture(int cameraId) async {
    final image = await methodsChannel.invokeMethod<String?>(
      CameraAuroraMethods.takePicture.name,
      {'cameraId': cameraId},
    );
    if (image!.isEmpty) {
      throw CameraException('nv12', 'Empty image data!');
    }
    final bytes = base64Decode(image);
    return XFile.fromData(
      bytes,
      name: 'temp.jpg',
      mimeType: 'image/jpeg',
      length: bytes.length,
    );
  }
}
