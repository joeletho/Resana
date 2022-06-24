#pragma once

#include "core/Layer.h"

#include "helpers/Time.h"

namespace RESANA {

		enum RefreshRate {
		Fast = 500,
		Normal = 1000,
		Slow = 2000
	};

	class Panel : public Layer {
	public:
		~Panel() override = default;
		void OnAttach() override = 0;
		void OnDetach() override = 0;
		void OnUpdate(Timestep ts) override = 0;
		void OnImGuiRender() override = 0;
		virtual void ShowPanel(bool* pOpen) = 0;
		virtual void SetTickRate(Timestep tickRate) {};
	};

}