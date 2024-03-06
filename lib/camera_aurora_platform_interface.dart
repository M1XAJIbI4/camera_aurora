// SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause
import 'package:camera_platform_interface/camera_platform_interface.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'camera_aurora_method_channel.dart';
import 'camera_data.dart';

abstract class CameraAuroraPlatform extends PlatformInterface {
  /// Constructs a CameraAuroraPlatform.
  CameraAuroraPlatform() : super(token: _token);

  static final Object _token = Object();

  static CameraAuroraPlatform _instance = MethodChannelCameraAurora();

  /// The default instance of [CameraAuroraPlatform] to use.
  ///
  /// Defaults to [MethodChannelCameraAurora].
  static CameraAuroraPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [CameraAuroraPlatform] when
  /// they register themselves.
  static set instance(CameraAuroraPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Stream<CameraState> onChangeState() {
    throw UnimplementedError('onChangeState() has not been implemented.');
  }

  Stream<String> onChangeQr() {
    throw UnimplementedError('onChangeQr() has not been implemented.');
  }

  Future<void> resizeFrame(double width, double height) {
    throw UnimplementedError('resizeFrame() has not been implemented.');
  }

  Future<List<CameraDescription>> availableCameras() {
    throw UnimplementedError('availableCameras() has not been implemented.');
  }

  Future<void> startCapture(double width, double height) {
    throw UnimplementedError('startCapture() has not been implemented.');
  }

  Future<void> stopCapture() {
    throw UnimplementedError('startCapture() has not been implemented.');
  }

  Future<CameraState> createCamera(String cameraName) {
    throw UnimplementedError('createCamera() has not been implemented.');
  }

  Future<void> dispose() {
    throw UnimplementedError('dispose() has not been implemented.');
  }

  Future<XFile> takePicture(int cameraId) {
    throw UnimplementedError('takePicture() has not been implemented.');
  }
}
