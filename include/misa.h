#ifndef MISA_H
#define MISA_H

#include <QObject>

class Misa : public QObject
{
    Q_OBJECT
public:
    explicit Misa(QObject *parent = nullptr);
    void MisaInit(quint32 misa);

    quint32 a  ;                         /*!< bit:     0  Atomic extension */
    quint32 b  ;                         /*!< bit:     1  Tentatively reserved for Bit-Manipulation extension */
    quint32 c  ;                         /*!< bit:     2  Compressed extension */
    quint32 d  ;                         /*!< bit:     3  Double-precision floating-point extension */
    quint32 e  ;                         /*!< bit:     4  RV32E base ISA */
    quint32 f  ;                         /*!< bit:     5  Single-precision floating-point extension */
    quint32 g  ;                         /*!< bit:     6  Additional standard extensions present */
    quint32 h  ;                         /*!< bit:     7  Hypervisor extension */
    quint32 i  ;                         /*!< bit:     8  RV32I/64I/128I base ISA */
    quint32 j  ;                         /*!< bit:     9  Tentatively reserved for Dynamically Translated Languages extension */
    quint32 l  ;                         /*!< bit:     11 Tentatively reserved for Decimal Floating-Point extension  */
    quint32 m  ;                         /*!< bit:     12 Integer Multiply/Divide extension */
    quint32 n  ;                         /*!< bit:     13 User-level interrupts supported  */
    quint32 p  ;                         /*!< bit:     15 Tentatively reserved for Packed-SIMD extension  */
    quint32 q  ;                         /*!< bit:     16 Quad-precision floating-point extension  */
    quint32 s  ;                         /*!< bit:     18 Supervisor mode implemented  */
    quint32 t  ;                         /*!< bit:     19 Tentatively reserved for Transactional Memory extension  */
    quint32 u  ;                         /*!< bit:     20 User mode implemented  */
    quint32 v  ;                         /*!< bit:     21 Tentatively reserved for Vector extension  */
    quint32 x  ;                         /*!< bit:     23 Non-standard extensions present  */
    quint32 mxl;                         /*!< bit:     30..31 Machine XLEN  */

signals:

};

#endif // MISA_H
