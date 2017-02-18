#pragma once

#include <ida.hpp>

#undef __on_failure
#undef __useHeader

#ifdef __QT5__
#include <QtWidgets/QtWidgets>
#else
#include <QtGui/QtGui>
//#include <QtGui/QWidget>
#endif

#define VDP_TILE_W 8
#define VDP_TILE_H 8
#define VDP_TILE_ZOOM 4

static const uint16 pal_raw[16] =
{
    0x0CCC, 0x0444, 0x0886, 0x0CAA, 0x0026, 0x022C, 0x026C, 0x02CC,
    0x0220, 0x0240, 0x0C86, 0x0420, 0x0A42, 0x0462, 0x08AC, 0x0000,
};

class PaintForm : public QWidget
{
    Q_OBJECT

        ea_t prevEA = BADADDR;
public:
    // PaintForm();
protected:
    void paintEvent(QPaintEvent *event);
signals:
    public slots:

};