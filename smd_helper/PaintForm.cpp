#include <ida.hpp>
#include <kernwin.hpp>
#include <bytes.hpp>

#include "PaintForm.h"

#define VDP_TILES_IN_ROW (this->width() / (VDP_TILE_W * VDP_TILE_ZOOM))
#define VDP_TILES_IN_COL (this->height() / (VDP_TILE_H * VDP_TILE_ZOOM))

void PaintForm::paintEvent(QPaintEvent *event)
{
    ea_t start = get_screen_ea();

    if (start == BADADDR || start == prevEA)
        return;

    ea_t end = start + VDP_TILES_IN_ROW * VDP_TILES_IN_COL;
    ea_t tiles = end - start;

    QPainter painter(this);
    painter.begin(this);

    QPen myPen(Qt::red, VDP_TILE_ZOOM, Qt::SolidLine);
    painter.setPen(myPen);

    QColor palette[16];

    for (int i = 0; i < _countof(palette); ++i)
    {
        uint8 r = (uint8)(((pal_raw[i] >> 0) & 0xE) << 4);
        uint8 g = (uint8)(((pal_raw[i] >> 4) & 0xE) << 4);
        uint8 b = (uint8)(((pal_raw[i] >> 8) & 0xE) << 4);
        palette[i] = QColor(r, g, b);
    }

    for (ea_t i = 0; i < tiles; ++i)
    {
        for (int y = 0; y < VDP_TILE_H; ++y)
        {
            for (int x = 0; x < (VDP_TILE_W / 2); ++x)
            {
                int _x = (i % VDP_TILES_IN_ROW) * VDP_TILE_W + x * 2;
                int _y = (i / VDP_TILES_IN_ROW) * VDP_TILE_H + y;

                uchar t = get_byte(start + i * 0x20 + y * (VDP_TILE_W / 2) + x);

                myPen.setColor(palette[t >> 4]);
                painter.setPen(myPen);
                painter.drawPoint((_x + 0) * VDP_TILE_ZOOM, _y * VDP_TILE_ZOOM);

                myPen.setColor(palette[t & 0x0F]);
                painter.setPen(myPen);
                painter.drawPoint((_x + 1) * VDP_TILE_ZOOM, _y * VDP_TILE_ZOOM);
            }
        }
    }

    painter.end();
}

#undef VDP_TILES_IN_COL
#undef VDP_TILES_IN_ROW