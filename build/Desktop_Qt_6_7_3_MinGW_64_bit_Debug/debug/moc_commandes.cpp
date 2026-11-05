/****************************************************************************
** Meta object code from reading C++ file 'commandes.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../commandes.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'commandes.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSCommandesENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSCommandesENDCLASS = QtMocHelpers::stringData(
    "Commandes",
    "on_pushButton_ajouter_clicked",
    "",
    "on_pushButton_modifier_clicked",
    "on_pushButton_supprimer_clicked",
    "on_pushButton_vider_clicked",
    "on_pushButton_trierDate_clicked",
    "on_pushButton_exportCSV_clicked",
    "on_pushButton_majStatutAuto_clicked",
    "on_tableWidget_commandes_cellClicked",
    "row",
    "column",
    "on_lineEdit_recherche_textChanged",
    "arg1",
    "on_comboBox_filtreStatut_currentIndexChanged",
    "index"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCommandesENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x08,    1 /* Private */,
       3,    0,   75,    2, 0x08,    2 /* Private */,
       4,    0,   76,    2, 0x08,    3 /* Private */,
       5,    0,   77,    2, 0x08,    4 /* Private */,
       6,    0,   78,    2, 0x08,    5 /* Private */,
       7,    0,   79,    2, 0x08,    6 /* Private */,
       8,    0,   80,    2, 0x08,    7 /* Private */,
       9,    2,   81,    2, 0x08,    8 /* Private */,
      12,    1,   86,    2, 0x08,   11 /* Private */,
      14,    1,   89,    2, 0x08,   13 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   10,   11,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::Int,   15,

       0        // eod
};

Q_CONSTINIT const QMetaObject Commandes::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSCommandesENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCommandesENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCommandesENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Commandes, std::true_type>,
        // method 'on_pushButton_ajouter_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButton_modifier_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButton_supprimer_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButton_vider_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButton_trierDate_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButton_exportCSV_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButton_majStatutAuto_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_tableWidget_commandes_cellClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'on_lineEdit_recherche_textChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'on_comboBox_filtreStatut_currentIndexChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void Commandes::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Commandes *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->on_pushButton_ajouter_clicked(); break;
        case 1: _t->on_pushButton_modifier_clicked(); break;
        case 2: _t->on_pushButton_supprimer_clicked(); break;
        case 3: _t->on_pushButton_vider_clicked(); break;
        case 4: _t->on_pushButton_trierDate_clicked(); break;
        case 5: _t->on_pushButton_exportCSV_clicked(); break;
        case 6: _t->on_pushButton_majStatutAuto_clicked(); break;
        case 7: _t->on_tableWidget_commandes_cellClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 8: _t->on_lineEdit_recherche_textChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->on_comboBox_filtreStatut_currentIndexChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *Commandes::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Commandes::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCommandesENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Commandes::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
