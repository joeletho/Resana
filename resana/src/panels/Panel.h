#pragma once

namespace RESANA {

    class Panel {
    public:
        virtual ~Panel() = default;

        virtual void ShowPanel(bool* pOpen) = 0;
    };

}