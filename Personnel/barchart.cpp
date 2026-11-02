#include "barchart.h"
#include <QPainter>

BarChart::BarChart(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(140);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void BarChart::setData(const QStringList &libelles, const QList<int> &valeurs)
{
    m_libelles = libelles;
    m_valeurs = valeurs;
    update();
}

void BarChart::paintEvent(QPaintEvent *)
{
    if (m_valeurs.isEmpty())
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int gauche = 30, haut = 20, droite = 6, bas = 22;
    const QRect zone(gauche, haut, width() - gauche - droite, height() - haut - bas);
    // Échelle adaptée aux valeurs (au moins 0-25, puis s'agrandit avec l'effectif)
    int valeurMax = 0;
    for (int v : m_valeurs)
        valeurMax = qMax(valeurMax, v);
    int maxi = qMax(25, ((valeurMax + 4) / 5) * 5);
    const int pas = (maxi <= 25) ? 5 : (((maxi / 5) + 4) / 5) * 5;
    maxi = ((maxi + pas - 1) / pas) * pas;

    QFont police = font();
    police.setPointSize(8);
    p.setFont(police);

    // Lignes de repère et graduations (0, 5, 10 ... 25)
    for (int v = 0; v <= maxi; v += pas) {
        const int y = zone.bottom() - zone.height() * v / maxi;
        p.setPen(QColor("#E3EDF3"));
        p.drawLine(zone.left(), y, zone.right(), y);
        p.setPen(QColor("#667788"));
        p.drawText(QRect(0, y - 8, gauche - 6, 16), Qt::AlignRight | Qt::AlignVCenter, QString::number(v));
    }

    // Barres
    const int n = m_valeurs.size();
    const int largeurCase = zone.width() / n;
    const int largeurBarre = int(largeurCase * 0.55);

    for (int i = 0; i < n; ++i) {
        const int h = zone.height() * m_valeurs[i] / maxi;
        const int x = zone.left() + i * largeurCase + (largeurCase - largeurBarre) / 2;
        const QRect barre(x, zone.bottom() - h, largeurBarre, h);
        p.fillRect(barre, QColor(i % 2 == 0 ? "#0A4D8C" : "#00B4D8"));

        p.setPen(QColor("#1b2a38"));
        QFont gras = police;
        gras.setBold(true);
        p.setFont(gras);
        p.drawText(QRect(barre.left() - 10, barre.top() - 16, largeurBarre + 20, 14), Qt::AlignCenter, QString::number(m_valeurs[i]));

        p.setFont(police);
        p.setPen(QColor("#667788"));
        p.drawText(QRect(zone.left() + i * largeurCase, zone.bottom() + 4, largeurCase, 16), Qt::AlignCenter, m_libelles.value(i));
    }
}
