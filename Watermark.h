#ifndef WATERMARK_H
#define WATERMARK_H

#include <QMainWindow>

class Ui_WatermarkClass;

class Watermark : public QMainWindow {
    Q_OBJECT

public:
    Watermark(QWidget* parent = nullptr);
    ~Watermark();

private slots:
    void onChooseButtonClicked();
    void onApplyButtonClicked();
    void onSaveButtonClicked();
    void onColorButtonClicked();

    void onTemplate1ButtonClicked();
    void onTemplate2ButtonClicked();
    void onTemplate3ButtonClicked();


private:
    Ui_WatermarkClass* ui;

    QImage image;
    QImage editedImage;
    QImage textImage;
    QImage background;
    QColor color;

    void apply();

    QImage addMargin(const QImage image, const QImage background,
                     double xMarginRatio, double yMarginRatio,
                     double centerXRatio, double centerYRatio);

    QImage blurBackground(const QImage image);

    QImage roundCorners(const QImage image, double radiusR);

    QImage addText(QImage image, QString text, QString font, double sizeR, bool isBold, bool isWhite, double xR, double yR);


protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif // WATERMARK_H
