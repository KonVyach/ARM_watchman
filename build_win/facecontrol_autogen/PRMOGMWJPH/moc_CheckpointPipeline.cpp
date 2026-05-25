/****************************************************************************
** Meta object code from reading C++ file 'CheckpointPipeline.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/core/CheckpointPipeline.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CheckpointPipeline.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN2fc18CheckpointPipelineE_t {};
} // unnamed namespace

template <> constexpr inline auto fc::CheckpointPipeline::qt_create_metaobjectdata<qt_meta_tag_ZN2fc18CheckpointPipelineE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "fc::CheckpointPipeline",
        "stateChanged",
        "",
        "CheckpointState",
        "state",
        "facesCaptured",
        "QImage",
        "face",
        "faceMatched",
        "Person",
        "person",
        "similarity",
        "faceNotMatched",
        "bestSimilarity",
        "metalReadingReceived",
        "MetalDetectorReading",
        "reading",
        "temperatureReceived",
        "celsius",
        "intoxicationReceived",
        "IntoxicationReading",
        "granted",
        "denied",
        "DenyReason",
        "reason",
        "details",
        "operatorActionRequired",
        "message",
        "diagnostic"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'stateChanged'
        QtMocHelpers::SignalData<void(CheckpointState)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'facesCaptured'
        QtMocHelpers::SignalData<void(const QImage &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 },
        }}),
        // Signal 'faceMatched'
        QtMocHelpers::SignalData<void(const Person &, double)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 }, { QMetaType::Double, 11 },
        }}),
        // Signal 'faceNotMatched'
        QtMocHelpers::SignalData<void(double)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 13 },
        }}),
        // Signal 'metalReadingReceived'
        QtMocHelpers::SignalData<void(const MetalDetectorReading &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 15, 16 },
        }}),
        // Signal 'temperatureReceived'
        QtMocHelpers::SignalData<void(double)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 18 },
        }}),
        // Signal 'intoxicationReceived'
        QtMocHelpers::SignalData<void(const IntoxicationReading &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 20, 16 },
        }}),
        // Signal 'granted'
        QtMocHelpers::SignalData<void(const Person &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
        // Signal 'denied'
        QtMocHelpers::SignalData<void(DenyReason, const QString &)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 23, 24 }, { QMetaType::QString, 25 },
        }}),
        // Signal 'operatorActionRequired'
        QtMocHelpers::SignalData<void(CheckpointState, const QString &)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::QString, 27 },
        }}),
        // Signal 'diagnostic'
        QtMocHelpers::SignalData<void(const QString &)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 27 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CheckpointPipeline, qt_meta_tag_ZN2fc18CheckpointPipelineE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject fc::CheckpointPipeline::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2fc18CheckpointPipelineE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2fc18CheckpointPipelineE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN2fc18CheckpointPipelineE_t>.metaTypes,
    nullptr
} };

void fc::CheckpointPipeline::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CheckpointPipeline *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stateChanged((*reinterpret_cast<std::add_pointer_t<CheckpointState>>(_a[1]))); break;
        case 1: _t->facesCaptured((*reinterpret_cast<std::add_pointer_t<QImage>>(_a[1]))); break;
        case 2: _t->faceMatched((*reinterpret_cast<std::add_pointer_t<Person>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        case 3: _t->faceNotMatched((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 4: _t->metalReadingReceived((*reinterpret_cast<std::add_pointer_t<MetalDetectorReading>>(_a[1]))); break;
        case 5: _t->temperatureReceived((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 6: _t->intoxicationReceived((*reinterpret_cast<std::add_pointer_t<IntoxicationReading>>(_a[1]))); break;
        case 7: _t->granted((*reinterpret_cast<std::add_pointer_t<Person>>(_a[1]))); break;
        case 8: _t->denied((*reinterpret_cast<std::add_pointer_t<DenyReason>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->operatorActionRequired((*reinterpret_cast<std::add_pointer_t<CheckpointState>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->diagnostic((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(CheckpointState )>(_a, &CheckpointPipeline::stateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(const QImage & )>(_a, &CheckpointPipeline::facesCaptured, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(const Person & , double )>(_a, &CheckpointPipeline::faceMatched, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(double )>(_a, &CheckpointPipeline::faceNotMatched, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(const MetalDetectorReading & )>(_a, &CheckpointPipeline::metalReadingReceived, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(double )>(_a, &CheckpointPipeline::temperatureReceived, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(const IntoxicationReading & )>(_a, &CheckpointPipeline::intoxicationReceived, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(const Person & )>(_a, &CheckpointPipeline::granted, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(DenyReason , const QString & )>(_a, &CheckpointPipeline::denied, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(CheckpointState , const QString & )>(_a, &CheckpointPipeline::operatorActionRequired, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (CheckpointPipeline::*)(const QString & )>(_a, &CheckpointPipeline::diagnostic, 10))
            return;
    }
}

const QMetaObject *fc::CheckpointPipeline::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *fc::CheckpointPipeline::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN2fc18CheckpointPipelineE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int fc::CheckpointPipeline::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void fc::CheckpointPipeline::stateChanged(CheckpointState _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void fc::CheckpointPipeline::facesCaptured(const QImage & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void fc::CheckpointPipeline::faceMatched(const Person & _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void fc::CheckpointPipeline::faceNotMatched(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void fc::CheckpointPipeline::metalReadingReceived(const MetalDetectorReading & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void fc::CheckpointPipeline::temperatureReceived(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void fc::CheckpointPipeline::intoxicationReceived(const IntoxicationReading & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void fc::CheckpointPipeline::granted(const Person & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void fc::CheckpointPipeline::denied(DenyReason _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1, _t2);
}

// SIGNAL 9
void fc::CheckpointPipeline::operatorActionRequired(CheckpointState _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2);
}

// SIGNAL 10
void fc::CheckpointPipeline::diagnostic(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}
QT_WARNING_POP
