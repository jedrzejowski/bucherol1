//
// Created by adam on 08.05.2021.
//

#pragma once

#include "./_def.h"

namespace SokarAlg {

	class ValueInterpolator {
	protected:
		const DicomVolume *vv = nullptr;

		virtual void dicomVolumeChanged();

	public:
		virtual ~ValueInterpolator();

		[[nodiscard]]
		const DicomVolume *getVolume() const;
		void setVolume(const DicomVolume *vv);

		[[nodiscard]]
		virtual float interpolate(const glm::vec3 &position) const = 0;
	};

	class NearestValueInterpolator : public ValueInterpolator {
	public:
		[[nodiscard]]
		float interpolate(const glm::vec3 &position) const override;
	};

	class LinearValueInterpolator : public ValueInterpolator {
	public:
		[[nodiscard]]
		float interpolate(const glm::vec3 &position) const override;
	};

	class PolynomialValueInterpolator1 : public ValueInterpolator {
	protected:
		glm::i32vec3 size = glm::i32vec3(2, 2, 2);
	public:

		[[nodiscard]]
		const glm::i32vec3 &getSize() const;
		void setSize(const glm::i32vec3 &size);

		[[nodiscard]]
		float interpolate(const glm::vec3 &position) const override;
	};

	class PolynomialValueInterpolator2 : public PolynomialValueInterpolator1 {
	public:

		[[nodiscard]]
		float interpolate(const glm::vec3 &position) const override;
	};



	//https://www.mathworks.com/help/matlab/ref/interp3.html
}



