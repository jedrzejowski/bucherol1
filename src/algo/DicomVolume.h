//
// Created by adam on 14.03.2021.
//

#pragma once

#include "./_def.h"
#include "../_classdef.h"
#include "./VirtualVolume.h"
#include "../dicomview/scenes/sets/_sceneset.h"

namespace SokarAlg {

	class DicomVolume : public VirtualVolume {

		const Sokar::DicomSceneSet *sceneSet = nullptr;

	public:

		[[nodiscard]]
		const Sokar::DicomSceneSet *getSceneSet() const;

		void setSceneSet(const Sokar::DicomSceneSet *sceneSet);

		[[nodiscard]]
		glm::u32vec3 getSize() const override;

		[[nodiscard]]
		float getValue(const quint32 &x, const quint32 &y, const quint32 &z) const override;
	};
}


