/****************************************************************************
** Meta object code from reading C++ file 'MockMetalDetector.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/hardware/MockMetalDetector.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MockMetalDetector.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
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
struct qt_meta_tag_ZN2fc17MockMetalDetectorE_t {};
} // unnamed namespace

template <> constexpr inline auto fc::MockMetalDetector::qt_create_metaobjectdata<qt_meta_tag_ZN2fc17MockMetalDetectorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "fc::MockMetalDetector",
        "setSimulatedTriggered",
        "",
        "triggered",
        "setSimulatedItems",
        "items"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'setSimulatedTriggered'
        QtMocHelpers::SlotData<void(bool)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'setSimulatedItems'
        QtMocHelpers::SlotData<void(const QStringList &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QStringList, 5 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MockMetalDetector, qt_meta_tag_ZN2fc17MockMetalDetectorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject fc::MockMetalDetector::staticMetaObject = { {
    QMetaObject::SuperData::link<IMetalDetector::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2fc17MockMetalDetectorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2fc17MockMetalDetectorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN2fc17MockMetalDetectorE_t>.metaTypes,
    nullptr
} };

void fc::MockMetalDetector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MockMetalDetector *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->setSimulatedTriggered((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: _t->setSimulatedItems((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *fc::MockMetalDetector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *fc::MockMetalDetector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2fc17MockMetalDetectorE_t>.strings))
        return static_cast<void*>(this);
    return IMetalDetector::qt_metacast(_clname);
}

int fc::MockMetalDetector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = IMetalDetector::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
