#include "QPushButtonAlter.h"

/**
* Capture the mouse release event and emit the rightClicked signal.
*
* @param e The The mouse event.
*/
void QPushButtonAlter::mouseReleaseEvent(QMouseEvent *e){

    if (e->button() == Qt::RightButton){
        emit rightClicked();
    } else {
        if (e->button() == Qt::LeftButton){
            emit clicked();
        }
    }
}

/**
* Capture the mouse double click event and emit the leftDoubleClicked signal.
*
* @param e The The mouse event.
*/
void QPushButtonAlter::mouseDoubleClickEvent(QMouseEvent *e){
    if(e->button() == Qt::LeftButton){
        emit leftDoubleClicked();
    }else{
        if(e->button() == Qt::LeftButton){
            emit clicked();
        }
    }
}

void QPushButtonAlter::prepare(QString const& imageFilename){

    QSize icoSize(115, 115);
    QSize btnSize(120, 120);

    Image *i = Util::openThumbnail(imageFilename);
    QImage *img;
    QIcon ico;
    img = Util::convertImageToQImage(i);
    ico.addPixmap(QPixmap::fromImage(img->scaled(btnSize)));
    delete (img);
    delete (i);

    setIcon(ico);
    setIconSize(icoSize);
    setFlat(true);
    resize(btnSize);
}
