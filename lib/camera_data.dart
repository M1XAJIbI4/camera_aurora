// SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause
enum OrientationEvent {
  undefined,
  portrait,
  landscape,
  portraitFlipped,
  landscapeFlipped,
}

class CameraState {
  CameraState.fromJson(Map<dynamic, dynamic> json)
      : id = json['id'] ?? "",
        textureId = json['textureId'] ?? -1,
        width = (json['width'] ?? 0).toDouble(),
        height = (json['height'] ?? 0).toDouble(),
        mountAngle = json['mountAngle'] ?? 0,
        rotationDisplay = json['rotationDisplay'] ?? 0,
        error = json['error'] ?? '';

  final String id;
  final int textureId;
  final double width;
  final double height;
  final int mountAngle;
  final int rotationDisplay;
  final String error;

  bool isNotEmpty() => textureId != -1;

  bool hasError() => error.isNotEmpty;

  @override
  String toString() {
    return '{id: $id, textureId: $textureId, width: $width, height: $height, mountAngle: $mountAngle, rotationDisplay: $rotationDisplay, error: $error}';
  }
}
