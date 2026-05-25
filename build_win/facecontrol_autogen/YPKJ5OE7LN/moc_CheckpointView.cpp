/****************************************************************************
** Meta object code from reading C++ file 'CheckpointView.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/ui/CheckpointView.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CheckpointView.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN2fc14CheckpointViewE_t {};
} // unnamed namespace

template <> constexpr inline auto fc::CheckpointView::qt_create_metaobjectdata<qt_meta_tag_ZN2fc14CheckpointViewE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "fc::CheckpointView",
        "onStateChanged",
        "",
        "CheckpointState",
        "s",
        "onFaceMatched",
        "Person",
        "p",
        "sim",
        "onFaceNotMatched",
        "onGranted",
        "onDenied",
        "DenyReason",
        "reason",
        "details",
        "onOperatorAction",
        "state",
        "message",
        "onDiagnostic",
        "msg",
        "onMetalReading",
        "MetalDetectorReading",
        "r",
        "onTempReading",
        "celsius",
        "onIntoxReading",
        "IntoxicationReading"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onStateChanged'
        QtMocHelpers::SlotData<void(CheckpointState)>(1, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Slot 'onFaceMatched'
        QtMocHelpers::SlotData<void(const Person &, double)>(5, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 6, 7 }, { QMetaType::Double, 8 },
        }}),
        // Slot 'onFaceNotMatched'
        QtMocHelpers::SlotData<void(double)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 8 },
        }}),
        // Slot 'onGranted'
        QtMocHelpers::SlotData<void(const Person &)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 6, 7 },
        }}),
        // Slot 'onDenied'
        QtMocHelpers::SlotData<void(DenyReason, const QString &)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 12, 13 }, { QMetaType::QString, 14 },
        }}),
        // Slot 'onOperatorAction'
        QtMocHelpers::SlotData<void(CheckpointState, const QString &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 3, 16 }, { QMetaType::QString, 17 },
        }}),
        // Slot 'onDiagnostic'
        QtMocHelpers::SlotData<void(const QString &)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Slot 'onMetalReading'
        QtMocHelpers::SlotData<void(const MetalDetectorReading &)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 21, 22 },
        }}),
        // Slot 'onTempReading'
        QtMocHelpers::SlotData<void(double)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 24 },
        }}),
        // Slot 'onIntoxReading'
        QtMocHelpers::SlotData<void(const IntoxicationReading &)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 26, 22 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CheckpointView, qt_meta_tag_ZN2fc14CheckpointViewE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject fc::CheckpointView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2fc14CheckpointViewE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2fc14CheckpointViewE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN2fc14CheckpointViewE_t>.metaTypes,
    nullptr
} };

void fc::CheckpointView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CheckpointView *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onStateChanged((*reinterpret_cast<std::add_pointer_t<CheckpointState>>(_a[1]))); break;
        case 1: _t->onFaceMatched((*reinterpret_cast<std::add_pointer_t<Person>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 2: _t->onFaceNotMatched((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 3: _t->onGranted((*reinterpret_cast<std::add_pointer_t<Person>>(_a[1]))); break;
        case 4: _t->onDenied((*reinterpret_cast<std::add_pointer_t<DenyReason>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->onOperatorAction((*reinterpret_cast<std::add_pointer_t<CheckpointState>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 6: _t->onDiagnostic((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->onMetalReading((*reinterpret_cast<std::add_pointer_t<MetalDetectorReading>>(_a[1]))); break;
        case 8: _t->onTempReading((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 9: _t->onIntoxReading((*reinterpret_cast<std::add_pointer_t<IntoxicationReading>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *fc::CheckpointView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *fc::CheckpointView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2fc14CheckpointViewE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int fc::CheckpointView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
