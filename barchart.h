#pragma once
#include <QWidget>
#include <QStringList>
#include <QList>

// Petit graphique en barres dessiné à la main (aucun module supplémentaire requis)
class BarChart : public QWidget
{
    Q_OBJECT
public:
    explicit BarChart(QWidget *parent = nullptr);
    void setData(const QStringList &libelles, const QList<int> &valeurs);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QStringList m_libelles;
    QList<int> m_valeurs;
};
