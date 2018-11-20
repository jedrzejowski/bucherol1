#pragma once

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsObject>

#include "_classdef.h"

class Sokar::Scene : public QGraphicsScene {
Q_OBJECT
protected:
	std::vector<SceneIndicator *> indicators;

public:

	explicit Scene(QWidget *parent = nullptr);

	~Scene() override;

	const std::vector<SceneIndicator *> &allIndicators() const{
		return indicators;
	}

	size_t indicatorsCount(){
		return indicators.size();
	}

	void addIndicator(SceneIndicator *item);

	bool removeIndicator(SceneIndicator *item);

};
