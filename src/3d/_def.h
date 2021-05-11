#pragma once

#include <QtGlobal>
#include <QVulkanInstance>
#include <QVulkanWindowRenderer>
#include <QVector3D>
#include <QVector2D>

#include "../sokar_glm.h"


namespace Sokar3D {

//	typedef glm::vec3 vec3;
//	typedef glm::vec2 vec2;

	static const float ESP = 0.00001f;

	struct MeshVertex;

	class Camera;

	class GameCamera;

	class CenterCamera;

	struct SimpleLight;

	struct SolidMaterial;

	class StaticMesh;

	class IndexedStaticMesh;

	class VulkanRenderer;

	class Shader;

	struct ShaderData;

	class VulkanWidget;

	class TestWindow;

	class PipelineWrapper;

	class MeshPipeline;

	QVulkanInstance *getVulkanInstance();

	static inline VkDeviceSize makeBuffSizeAligned(VkDeviceSize v, VkDeviceSize byteAlign) {
		return (v + byteAlign - 1) & ~(byteAlign - 1);
	}
}