// ---------------------------------------------------------------------------
//
// This file is part of:
//
// FRIDA - FRee Interactive DisAssembler
// Copyright (C) 2017,2023 Ivo van Poorten
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; ONLY version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ---------------------------------------------------------------------------

#ifndef LOWANDHIGHBYTEPAIRSWINDOW_H
#define LOWANDHIGHBYTEPAIRSWINDOW_H

#include "pch.h"

namespace Ui {
class LowAndHighBytePairsWindow;
}

class LowAndHighBytePairsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LowAndHighBytePairsWindow(QWidget *parent = nullptr);
    ~LowAndHighBytePairsWindow() override;

    bool pairsLowLowHighHigh{};
    bool minusLabels{};

    enum {
        NoLabels        = 0,
        LocalLabels     = 1,
        GlobalLabels    = 2
    };

    int generateLabels{};

public Q_SLOTS:
    void accept() override;
    void onNoLabels_toggled();

private:
    Ui::LowAndHighBytePairsWindow *ui;
};

#endif // LOWANDHIGHBYTEPAIRSWINDOW_H
