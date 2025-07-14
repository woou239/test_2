/****************************************************************************
** Meta object code from reading C++ file 'multiThread.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../multiThread.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'multiThread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.6.1. It"
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
struct qt_meta_stringdata_CLASSIndicatorLightENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSIndicatorLightENDCLASS = QtMocHelpers::stringData(
    "IndicatorLight"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSIndicatorLightENDCLASS_t {
    uint offsetsAndSizes[2];
    char stringdata0[15];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSIndicatorLightENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSIndicatorLightENDCLASS_t qt_meta_stringdata_CLASSIndicatorLightENDCLASS = {
    {
        QT_MOC_LITERAL(0, 14)   // "IndicatorLight"
    },
    "IndicatorLight"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSIndicatorLightENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject IndicatorLight::staticMetaObject = { {
    QMetaObject::SuperData::link<QPushButton::staticMetaObject>(),
    qt_meta_stringdata_CLASSIndicatorLightENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSIndicatorLightENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSIndicatorLightENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<IndicatorLight, std::true_type>
    >,
    nullptr
} };

void IndicatorLight::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *IndicatorLight::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IndicatorLight::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSIndicatorLightENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QPushButton::qt_metacast(_clname);
}

int IndicatorLight::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPushButton::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSGetDataThreadENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSGetDataThreadENDCLASS = QtMocHelpers::stringData(
    "GetDataThread",
    "trans_ringbuffer",
    "",
    "lpParam",
    "boards_nums"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSGetDataThreadENDCLASS_t {
    uint offsetsAndSizes[10];
    char stringdata0[14];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[12];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSGetDataThreadENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSGetDataThreadENDCLASS_t qt_meta_stringdata_CLASSGetDataThreadENDCLASS = {
    {
        QT_MOC_LITERAL(0, 13),  // "GetDataThread"
        QT_MOC_LITERAL(14, 16),  // "trans_ringbuffer"
        QT_MOC_LITERAL(31, 0),  // ""
        QT_MOC_LITERAL(32, 7),  // "lpParam"
        QT_MOC_LITERAL(40, 11)   // "boards_nums"
    },
    "GetDataThread",
    "trans_ringbuffer",
    "",
    "lpParam",
    "boards_nums"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSGetDataThreadENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   20,    2, 0x06,    1 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::VoidStar, QMetaType::Int,    3,    4,

       0        // eod
};

Q_CONSTINIT const QMetaObject GetDataThread::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_CLASSGetDataThreadENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSGetDataThreadENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSGetDataThreadENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<GetDataThread, std::true_type>,
        // method 'trans_ringbuffer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<void *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void GetDataThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GetDataThread *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->trans_ringbuffer((*reinterpret_cast< std::add_pointer_t<void*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GetDataThread::*)(void * , int );
            if (_t _q_method = &GetDataThread::trans_ringbuffer; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *GetDataThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GetDataThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSGetDataThreadENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int GetDataThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void GetDataThread::trans_ringbuffer(void * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSDoUpdateThreadENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDoUpdateThreadENDCLASS = QtMocHelpers::stringData(
    "DoUpdateThread",
    "update_customPlot",
    "",
    "double**",
    "real_time_data",
    "double*",
    "FFT_keys",
    "FFT_values",
    "energy_accumulation",
    "uint32_t",
    "length",
    "is_changed_xAxis",
    "mode",
    "do_trans_ringbuffer",
    "lpParam",
    "boards_nums",
    "do_fft_board_changed",
    "value",
    "do_fft_channel_changed",
    "do_energy_board_changed",
    "do_xAxis_changed"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDoUpdateThreadENDCLASS_t {
    uint offsetsAndSizes[42];
    char stringdata0[15];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[15];
    char stringdata5[8];
    char stringdata6[9];
    char stringdata7[11];
    char stringdata8[20];
    char stringdata9[9];
    char stringdata10[7];
    char stringdata11[17];
    char stringdata12[5];
    char stringdata13[20];
    char stringdata14[8];
    char stringdata15[12];
    char stringdata16[21];
    char stringdata17[6];
    char stringdata18[23];
    char stringdata19[24];
    char stringdata20[17];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDoUpdateThreadENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDoUpdateThreadENDCLASS_t qt_meta_stringdata_CLASSDoUpdateThreadENDCLASS = {
    {
        QT_MOC_LITERAL(0, 14),  // "DoUpdateThread"
        QT_MOC_LITERAL(15, 17),  // "update_customPlot"
        QT_MOC_LITERAL(33, 0),  // ""
        QT_MOC_LITERAL(34, 8),  // "double**"
        QT_MOC_LITERAL(43, 14),  // "real_time_data"
        QT_MOC_LITERAL(58, 7),  // "double*"
        QT_MOC_LITERAL(66, 8),  // "FFT_keys"
        QT_MOC_LITERAL(75, 10),  // "FFT_values"
        QT_MOC_LITERAL(86, 19),  // "energy_accumulation"
        QT_MOC_LITERAL(106, 8),  // "uint32_t"
        QT_MOC_LITERAL(115, 6),  // "length"
        QT_MOC_LITERAL(122, 16),  // "is_changed_xAxis"
        QT_MOC_LITERAL(139, 4),  // "mode"
        QT_MOC_LITERAL(144, 19),  // "do_trans_ringbuffer"
        QT_MOC_LITERAL(164, 7),  // "lpParam"
        QT_MOC_LITERAL(172, 11),  // "boards_nums"
        QT_MOC_LITERAL(184, 20),  // "do_fft_board_changed"
        QT_MOC_LITERAL(205, 5),  // "value"
        QT_MOC_LITERAL(211, 22),  // "do_fft_channel_changed"
        QT_MOC_LITERAL(234, 23),  // "do_energy_board_changed"
        QT_MOC_LITERAL(258, 16)   // "do_xAxis_changed"
    },
    "DoUpdateThread",
    "update_customPlot",
    "",
    "double**",
    "real_time_data",
    "double*",
    "FFT_keys",
    "FFT_values",
    "energy_accumulation",
    "uint32_t",
    "length",
    "is_changed_xAxis",
    "mode",
    "do_trans_ringbuffer",
    "lpParam",
    "boards_nums",
    "do_fft_board_changed",
    "value",
    "do_fft_channel_changed",
    "do_energy_board_changed",
    "do_xAxis_changed"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDoUpdateThreadENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    7,   50,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      13,    2,   65,    2, 0x0a,    9 /* Public */,
      16,    1,   70,    2, 0x0a,   12 /* Public */,
      18,    1,   73,    2, 0x0a,   14 /* Public */,
      19,    1,   76,    2, 0x0a,   16 /* Public */,
      20,    1,   79,    2, 0x0a,   18 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 9, QMetaType::Bool, 0x80000000 | 9,    4,    6,    7,    8,   10,   11,   12,

 // slots: parameters
    QMetaType::Void, QMetaType::VoidStar, QMetaType::Int,   14,   15,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::Int,   17,

       0        // eod
};

Q_CONSTINIT const QMetaObject DoUpdateThread::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_CLASSDoUpdateThreadENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDoUpdateThreadENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDoUpdateThreadENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DoUpdateThread, std::true_type>,
        // method 'update_customPlot'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double * *, std::false_type>,
        QtPrivate::TypeAndForceComplete<double *, std::false_type>,
        QtPrivate::TypeAndForceComplete<double *, std::false_type>,
        QtPrivate::TypeAndForceComplete<double *, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        // method 'do_trans_ringbuffer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<void *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'do_fft_board_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'do_fft_channel_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'do_energy_board_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'do_xAxis_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void DoUpdateThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DoUpdateThread *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->update_customPlot((*reinterpret_cast< std::add_pointer_t<double**>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double*>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double*>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[7]))); break;
        case 1: _t->do_trans_ringbuffer((*reinterpret_cast< std::add_pointer_t<void*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 2: _t->do_fft_board_changed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->do_fft_channel_changed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->do_energy_board_changed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->do_xAxis_changed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DoUpdateThread::*)(double * * , double * , double * , double * , uint32_t , bool , uint32_t );
            if (_t _q_method = &DoUpdateThread::update_customPlot; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *DoUpdateThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DoUpdateThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDoUpdateThreadENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int DoUpdateThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void DoUpdateThread::update_customPlot(double * * _t1, double * _t2, double * _t3, double * _t4, uint32_t _t5, bool _t6, uint32_t _t7)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
