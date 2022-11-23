#ifndef QPUSHBUTTONALTER_H
#define QPUSHBUTTONALTER_H

//Qt includes
#include <QPushButton>
#include <QMouseEvent>
#include <Util.h>

class QPushButtonAlter : public QPushButton{ Q_OBJECT

    protected:
        void mouseReleaseEvent(QMouseEvent *e);
        void mouseDoubleClickEvent(QMouseEvent *e);

    public:
        explicit QPushButtonAlter(QObject *parent = 0){
        }

        void prepare(QString const& imageFilename);

    signals:
        void rightClicked();
        void leftDoubleClicked();

};

#endif // QPUSHBUTTONALTER_H
