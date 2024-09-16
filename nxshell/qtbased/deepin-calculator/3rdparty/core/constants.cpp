// This file is part of the SpeedCrunch project
// Copyright (C) 2007 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2007, 2009, 2011, 2013, 2016 @heldercorreia
// Copyright (C) 2009 Andreas Scherer <andreas_coder@freenet.de>
// Copyright (C) 2016 Hadrien Theveneau <theveneau@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

/*#include "src/core/constants.h"

#include "src/core/numberformatter.h"
#include "../math/hmath.h"

#include <QCoreApplication>

#include <algorithm>

static Constants* s_constantsInstance = nullptr;

static void s_deleteConstants()
{
    delete s_constantsInstance;
}

struct Constants::Private
{
    QStringList categories;
    QList<Constant> list;

    void populate();
    void retranslateText();
};

// UNCERTAINTY  and CHECKEC reserved for future use
#define PUSH_CONSTANT_CODATA(NAME,VALUE,UNIT,UNCERTAINTY,CHECKED) \
    c.value = QLatin1String(VALUE); \
    c.unit  = QString::fromUtf8(UNIT); \
    list << c;

#define PUSH_CONSTANT(NAME,VALUE,UNIT) \
    c.value = QLatin1String(VALUE); \
    c.unit  = QString::fromUtf8(UNIT); \
    list << c;

#define PUSH_CONSTANT_NO_UNIT(NAME,VALUE) \
    c.value = QLatin1String(VALUE); \
    list << c;

#define I18N_CONSTANT(NAME) \
    i->name = NAME; \
    i->category = cat; \
    ++i;

#define I18N_CONSTANT_DAYS(NAME) \
    i->unit = days; \
    I18N_CONSTANT(NAME)

void Constants::Private::populate()
{
    Constant c;

    // CODATA Internationally recommended values of the Fundamental Physical Constants
    // Constants not yet included, to be included in future releases (TODO)
    // From: http://physics.nist.gov/cuu/Constants/
    // Date: 2016-03-28

    //   Quantity                                                       Value                 Uncertainty           Unit
    // -----------------------------------------------------------------------------------------------------------------------------
    // alpha particle-electron mass ratio                          7294.299 541 36          0.000 000 24
    // alpha particle mass                                         6.644 657 230 e-27       0.000 000 082 e-27       kg
    // alpha particle mass energy equivalent                       5.971 920 097 e-10       0.000 000 073 e-10       J
    // alpha particle mass energy equivalent in MeV                3727.379 378             0.000 023                MeV
    // alpha particle mass in u                                    4.001 506 179 127        0.000 000 000 063        u
    // alpha particle molar mass                                   4.001 506 179 127 e-3    0.000 000 000 063 e-3    kg mol^-1
    // alpha particle-proton mass ratio                            3.972 599 689 07         0.000 000 000 36
    // Angstrom star                                               1.000 014 95 e-10        0.000 000 90 e-10        m
    // atomic mass constant energy equivalent                      1.492 418 062 e-10       0.000 000 018 e-10       J
    // atomic mass constant energy equivalent in MeV               931.494 0954             0.000 0057               MeV
    // atomic mass unit-electron volt relationship                 931.494 0954 e6          0.000 0057 e6            eV
    // atomic mass unit-hartree relationship                       3.423 177 6902 e7        0.000 000 0016 e7        E_h
    // atomic mass unit-hertz relationship                         2.252 342 7206 e23       0.000 000 0010 e23       Hz
    // atomic mass unit-inverse meter relationship                 7.513 006 6166 e14       0.000 000 0034 e14       m^-1
    // atomic mass unit-joule relationship                         1.492 418 062 e-10       0.000 000 018 e-10       J
    // atomic mass unit-kelvin relationship                        1.080 954 38 e13         0.000 000 62 e13         K
    // atomic mass unit-kilogram relationship                      1.660 539 040 e-27       0.000 000 020 e-27       kg
    // atomic unit of 1st hyperpolarizability                      3.206 361 329 e-53       0.000 000 020 e-53       C^3 m^3 J^-2
    // atomic unit of 2nd hyperpolarizability                      6.235 380 085 e-65       0.000 000 077 e-65       C^4 m^4 J^-3
    // atomic unit of action                                       1.054 571 800 e-34       0.000 000 013 e-34       J s
    // atomic unit of charge                                       1.602 176 6208 e-19      0.000 000 0098 e-19      C
    // atomic unit of charge density                               1.081 202 3770 e12       0.000 000 0067 e12       C m^-3
    // atomic unit of current                                      6.623 618 183 e-3        0.000 000 041 e-3        A
    // atomic unit of electric dipole mom.                         8.478 353 552 e-30       0.000 000 052 e-30       C m
    // atomic unit of electric field                               5.142 206 707 e11        0.000 000 032 e11        V m^-1
    // atomic unit of electric field gradient                      9.717 362 356 e21        0.000 000 060 e21        V m^-2
    // atomic unit of electric polarizability                      1.648 777 2731 e-41      0.000 000 0011 e-41      C^2 m^2 J^-1
    // atomic unit of electric potential                           27.211 386 02            0.000 000 17             V
    // atomic unit of electric quadrupole mom.                     4.486 551 484 e-40       0.000 000 028 e-40       C m^2
    // atomic unit of energy                                       4.359 744 650 e-18       0.000 000 054 e-18       J
    // atomic unit of force                                        8.238 723 36 e-8         0.000 000 10 e-8         N
    // atomic unit of length                                       0.529 177 210 67 e-10    0.000 000 000 12 e-10    m
    // atomic unit of mag. dipole mom.                             1.854 801 999 e-23       0.000 000 011 e-23       J T^-1
    // atomic unit of mag. flux density                            2.350 517 550 e5         0.000 000 014 e5         T
    // atomic unit of magnetizability                              7.891 036 5886 e-29      0.000 000 0090 e-29      J T^-2
    // atomic unit of mass                                         9.109 383 56 e-31        0.000 000 11 e-31        kg
    // atomic unit of mom.um                                       1.992 851 882 e-24       0.000 000 024 e-24       kg m s^-1
    // atomic unit of permittivity                                 1.112 650 056... e-10    (exact)                  F m^-1
    // atomic unit of time                                         2.418 884 326 509 e-17   0.000 000 000 014 e-17   s
    // atomic unit of velocity                                     2.187 691 262 77 e6      0.000 000 000 50 e6      m s^-1
    // Bohr magneton in eV/T                                       5.788 381 8012 e-5       0.000 000 0026 e-5       eV T^-1
    // Bohr magneton in Hz/T                                       13.996 245 042 e9        0.000 000 086 e9         Hz T^-1
    // Bohr magneton in inverse meters per tesla                   46.686 448 14            0.000 000 29             m^-1 T^-1
    // Bohr magneton in K/T                                        0.671 714 05             0.000 000 39             K T^-1
    // Boltzmann constant in eV/K                                  8.617 3303 e-5           0.000 0050 e-5           eV K^-1
    // Boltzmann constant in Hz/K                                  2.083 6612 e10           0.000 0012 e10           Hz K^-1
    // Boltzmann constant in inverse meters per kelvin             69.503 457               0.000 040                m^-1 K^-1
    // classical electron radius                                   2.817 940 3227 e-15      0.000 000 0019 e-15      m
    // Cu x unit                                                   1.002 076 97 e-13        0.000 000 28 e-13        m
    // deuteron-electron mag. mom. ratio                           -4.664 345 535 e-4       0.000 000 026 e-4
    // deuteron-electron mass ratio                                3670.482 967 85          0.000 000 13
    // deuteron g factor                                           0.857 438 2311           0.000 000 0048
    // deuteron mag. mom.                                          0.433 073 5040 e-26      0.000 000 0036 e-26      J T^-1
    // deuteron mag. mom. to Bohr magneton ratio                   0.466 975 4554 e-3       0.000 000 0026 e-3
    // deuteron mag. mom. to nuclear magneton ratio                0.857 438 2311           0.000 000 0048
    // deuteron mass                                               3.343 583 719 e-27       0.000 000 041 e-27       kg
    // deuteron mass energy equivalent                             3.005 063 183 e-10       0.000 000 037 e-10       J
    // deuteron mass energy equivalent in MeV                      1875.612 928             0.000 012                MeV
    // deuteron mass in u                                          2.013 553 212 745        0.000 000 000 040        u
    // deuteron molar mass                                         2.013 553 212 745 e-3    0.000 000 000 040 e-3    kg mol^-1
    // deuteron-neutron mag. mom. ratio                            -0.448 206 52            0.000 000 11
    // deuteron-proton mag. mom. ratio                             0.307 012 2077           0.000 000 0015
    // deuteron-proton mass ratio                                  1.999 007 500 87         0.000 000 000 19
    // deuteron rms charge radius                                  2.1413 e-15              0.0025 e-15              m
    // electron charge to mass quotient                            -1.758 820 024 e11       0.000 000 011 e11        C kg^-1
    // electron-deuteron mag. mom. ratio                           -2143.923 499            0.000 012
    // electron-deuteron mass ratio                                2.724 437 107 484 e-4    0.000 000 000 096 e-4
    // electron g factor                                           -2.002 319 304 361 82    0.000 000 000 000 52
    // electron gyromag. ratio                                     1.760 859 644 e11        0.000 000 011 e11        s^-1 T^-1
    // electron gyromag. ratio over 2 pi                           28 024.951 64            0.000 17                 MHz T^-1
    // electron-helion mass ratio                                  1.819 543 074 854 e-4    0.000 000 000 088 e-4
    // electron mag. mom.                                          -928.476 4620 e-26       0.000 0057 e-26          J T^-1
    // electron mag. mom. anomaly                                  1.159 652 180 91 e-3     0.000 000 000 26 e-3
    // electron mag. mom. to Bohr magneton ratio                   -1.001 159 652 180 91    0.000 000 000 000 26
    // electron mag. mom. to nuclear magneton ratio                -1838.281 972 34         0.000 000 17

    // electron mass energy equivalent                             8.187 105 65 e-14        0.000 000 10 e-14        J

    // electron mass in u                                          5.485 799 090 70 e-4     0.000 000 000 16 e-4     u
    // electron molar mass                                         5.485 799 090 70 e-7     0.000 000 000 16 e-7     kg mol^-1
    // electron-muon mag. mom. ratio                               206.766 9880             0.000 0046
    // electron-muon mass ratio                                    4.836 331 70 e-3         0.000 000 11 e-3
    // electron-neutron mag. mom. ratio                            960.920 50               0.000 23
    // electron-neutron mass ratio                                 5.438 673 4428 e-4       0.000 000 0027 e-4
    // electron-proton mag. mom. ratio                             -658.210 6866            0.000 0020
    // electron-proton mass ratio                                  5.446 170 213 52 e-4     0.000 000 000 52 e-4
    // electron-tau mass ratio                                     2.875 92 e-4             0.000 26 e-4
    // electron to alpha particle mass ratio                       1.370 933 554 798 e-4    0.000 000 000 045 e-4
    // electron to shielded helion mag. mom. ratio                 864.058 257              0.000 010
    // electron to shielded proton mag. mom. ratio                 -658.227 5971            0.000 0072
    // electron-triton mass ratio                                  1.819 200 062 203 e-4    0.000 000 000 084 e-4
    // electron volt-atomic mass unit relationship                 1.073 544 1105 e-9       0.000 000 0066 e-9       u
    // electron volt-hartree relationship                          3.674 932 248 e-2        0.000 000 023 e-2        E_h
    // electron volt-hertz relationship                            2.417 989 262 e14        0.000 000 015 e14        Hz
    // electron volt-inverse meter relationship                    8.065 544 005 e5         0.000 000 050 e5         m^-1
    // electron volt-joule relationship                            1.602 176 6208 e-19      0.000 000 0098 e-19      J
    // electron volt-kelvin relationship                           1.160 452 21 e4          0.000 000 67 e4          K
    // electron volt-kilogram relationship                         1.782 661 907 e-36       0.000 000 011 e-36       kg
    // elementary charge over h                                    2.417 989 262 e14        0.000 000 015 e14        A J^-1
    // hartree-atomic mass unit relationship                       2.921 262 3197 e-8       0.000 000 0013 e-8       u
    // hartree-electron volt relationship                          27.211 386 02            0.000 000 17             eV
    // hartree-hertz relationship                                  6.579 683 920 711 e15    0.000 000 000 039 e15    Hz
    // hartree-inverse meter relationship                          2.194 746 313 702 e7     0.000 000 000 013 e7     m^-1
    // hartree-joule relationship                                  4.359 744 650 e-18       0.000 000 054 e-18       J
    // hartree-kelvin relationship                                 3.157 7513 e5            0.000 0018 e5            K
    // hartree-kilogram relationship                               4.850 870 129 e-35       0.000 000 060 e-35       kg
    // helion-electron mass ratio                                  5495.885 279 22          0.000 000 27
    // helion g factor                                             -4.255 250 616           0.000 000 050
    // helion mag. mom.                                            -1.074 617 522 e-26      0.000 000 014 e-26       J T^-1
    // helion mag. mom. to Bohr magneton ratio                     -1.158 740 958 e-3       0.000 000 014 e-3
    // helion mag. mom. to nuclear magneton ratio                  -2.127 625 308           0.000 000 025
    // helion mass                                                 5.006 412 700 e-27       0.000 000 062 e-27       kg
    // helion mass energy equivalent                               4.499 539 341 e-10       0.000 000 055 e-10       J
    // helion mass energy equivalent in MeV                        2808.391 586             0.000 017                MeV
    // helion mass in u                                            3.014 932 246 73         0.000 000 000 12         u
    // helion molar mass                                           3.014 932 246 73 e-3     0.000 000 000 12 e-3     kg mol^-1
    // helion-proton mass ratio                                    2.993 152 670 46         0.000 000 000 29
    // hertz-atomic mass unit relationship                         4.439 821 6616 e-24      0.000 000 0020 e-24      u
    // hertz-electron volt relationship                            4.135 667 662 e-15       0.000 000 025 e-15       eV
    // hertz-hartree relationship                                  1.519 829 846 0088 e-16  0.000 000 000 0090 e-16  E_h
    // hertz-inverse meter relationship                            3.335 640 951... e-9     (exact)                  m^-1
    // hertz-joule relationship                                    6.626 070 040 e-34       0.000 000 081 e-34       J
    // hertz-kelvin relationship                                   4.799 2447 e-11          0.000 0028 e-11          K
    // hertz-kilogram relationship                                 7.372 497 201 e-51       0.000 000 091 e-51       kg
    // inverse fine-structure constant                             137.035 999 139          0.000 000 031
    // inverse meter-atomic mass unit relationship                 1.331 025 049 00 e-15    0.000 000 000 61 e-15    u
    // inverse meter-electron volt relationship                    1.239 841 9739 e-6       0.000 000 0076 e-6       eV
    // inverse meter-hartree relationship                          4.556 335 252 767 e-8    0.000 000 000 027 e-8    E_h
    // inverse meter-hertz relationship                            299 792 458              (exact)                  Hz
    // inverse meter-joule relationship                            1.986 445 824 e-25       0.000 000 024 e-25       J
    // inverse meter-kelvin relationship                           1.438 777 36 e-2         0.000 000 83 e-2         K
    // inverse meter-kilogram relationship                         2.210 219 057 e-42       0.000 000 027 e-42       kg
    // joule-atomic mass unit relationship                         6.700 535 363 e9         0.000 000 082 e9         u
    // joule-electron volt relationship                            6.241 509 126 e18        0.000 000 038 e18        eV
    // joule-hartree relationship                                  2.293 712 317 e17        0.000 000 028 e17        E_h
    // joule-hertz relationship                                    1.509 190 205 e33        0.000 000 019 e33        Hz
    // joule-inverse meter relationship                            5.034 116 651 e24        0.000 000 062 e24        m^-1
    // joule-kelvin relationship                                   7.242 9731 e22           0.000 0042 e22           K
    // joule-kilogram relationship                                 1.112 650 056... e-17    (exact)                  kg
    // kelvin-atomic mass unit relationship                        9.251 0842 e-14          0.000 0053 e-14          u
    // kelvin-electron volt relationship                           8.617 3303 e-5           0.000 0050 e-5           eV
    // kelvin-hartree relationship                                 3.166 8105 e-6           0.000 0018 e-6           E_h
    // kelvin-hertz relationship                                   2.083 6612 e10           0.000 0012 e10           Hz
    // kelvin-inverse meter relationship                           69.503 457               0.000 040                m^-1
    // kelvin-joule relationship                                   1.380 648 52 e-23        0.000 000 79 e-23        J
    // kelvin-kilogram relationship                                1.536 178 65 e-40        0.000 000 88 e-40        kg
    // kilogram-atomic mass unit relationship                      6.022 140 857 e26        0.000 000 074 e26        u
    // kilogram-electron volt relationship                         5.609 588 650 e35        0.000 000 034 e35        eV
    // kilogram-hartree relationship                               2.061 485 823 e34        0.000 000 025 e34        E_h
    // kilogram-hertz relationship                                 1.356 392 512 e50        0.000 000 017 e50        Hz
    // kilogram-inverse meter relationship                         4.524 438 411 e41        0.000 000 056 e41        m^-1
    // kilogram-joule relationship                                 8.987 551 787... e16     (exact)                  J
    // kilogram-kelvin relationship                                6.509 6595 e39           0.000 0037 e39           K
    // lattice parameter of silicon                                543.102 0504 e-12        0.000 0089 e-12          m
    // molar mass constant                                         1 e-3                    (exact)                  kg mol^-1
    // molar mass of carbon-12                                     12 e-3                   (exact)                  kg mol^-1
    // molar Planck constant times c                               0.119 626 565 582        0.000 000 000 054        J m mol^-1
    // molar volume of ideal gas (273.15 K, 100 kPa)               22.710 947 e-3           0.000 013 e-3            m^3 mol^-1
    // molar volume of ideal gas (273.15 K, 101.325 kPa)           22.413 962 e-3           0.000 013 e-3            m^3 mol^-1
    // molar volume of silicon                                     12.058 832 14 e-6        0.000 000 61 e-6         m^3 mol^-1
    // Mo x unit                                                   1.002 099 52 e-13        0.000 000 53 e-13        m
    // muon Compton wavelength                                     11.734 441 11 e-15       0.000 000 26 e-15        m
    // muon Compton wavelength over 2 pi                           1.867 594 308 e-15       0.000 000 042 e-15       m
    // muon-electron mass ratio                                    206.768 2826             0.000 0046
    // muon g factor                                               -2.002 331 8418          0.000 000 0013
    // muon mag. mom.                                              -4.490 448 26 e-26       0.000 000 10 e-26        J T^-1
    // muon mag. mom. anomaly                                      1.165 920 89 e-3         0.000 000 63 e-3
    // muon mag. mom. to Bohr magneton ratio                       -4.841 970 48 e-3        0.000 000 11 e-3
    // muon mag. mom. to nuclear magneton ratio                    -8.890 597 05            0.000 000 20
    // muon mass energy equivalent                                 1.692 833 774 e-11       0.000 000 043 e-11       J
    // muon mass in u                                              0.113 428 9257           0.000 000 0025           u
    // muon molar mass                                             0.113 428 9257 e-3       0.000 000 0025 e-3       kg mol^-1
    // muon-neutron mass ratio                                     0.112 454 5167           0.000 000 0025
    // muon-proton mag. mom. ratio                                 -3.183 345 142           0.000 000 071
    // muon-proton mass ratio                                      0.112 609 5262           0.000 000 0025
    // muon-tau mass ratio                                         5.946 49 e-2             0.000 54 e-2
    // natural unit of action                                      1.054 571 800 e-34       0.000 000 013 e-34       J s
    // natural unit of action in eV s                              6.582 119 514 e-16       0.000 000 040 e-16       eV s
    // natural unit of energy                                      8.187 105 65 e-14        0.000 000 10 e-14        J
    // natural unit of energy in MeV                               0.510 998 9461           0.000 000 0031           MeV
    // natural unit of length                                      386.159 267 64 e-15      0.000 000 18 e-15        m
    // natural unit of mass                                        9.109 383 56 e-31        0.000 000 11 e-31        kg
    // natural unit of mom.um                                      2.730 924 488 e-22       0.000 000 034 e-22       kg m s^-1
    // natural unit of mom.um in MeV/c                             0.510 998 9461           0.000 000 0031           MeV/c
    // natural unit of time                                        1.288 088 667 12 e-21    0.000 000 000 58 e-21    s
    // natural unit of velocity                                    299 792 458              (exact)                  m s^-1
    // neutron Compton wavelength                                  1.319 590 904 81 e-15    0.000 000 000 88 e-15    m
    // neutron Compton wavelength over 2 pi                        0.210 019 415 36 e-15    0.000 000 000 14 e-15    m
    // neutron-electron mag. mom. ratio                            1.040 668 82 e-3         0.000 000 25 e-3
    // neutron-electron mass ratio                                 1838.683 661 58          0.000 000 90
    // neutron g factor                                            -3.826 085 45            0.000 000 90
    // neutron gyromag. ratio                                      1.832 471 72 e8          0.000 000 43 e8          s^-1 T^-1
    // neutron gyromag. ratio over 2 pi                            29.164 6933              0.000 0069               MHz T^-1
    // neutron mag. mom.                                           -0.966 236 50 e-26       0.000 000 23 e-26        J T^-1
    // neutron mag. mom. to Bohr magneton ratio                    -1.041 875 63 e-3        0.000 000 25 e-3
    // neutron mag. mom. to nuclear magneton ratio                 -1.913 042 73            0.000 000 45
    // neutron mass energy equivalent                              1.505 349 739 e-10       0.000 000 019 e-10       J
    // neutron mass energy equivalent in MeV                       939.565 4133             0.000 0058               MeV
    // neutron mass in u                                           1.008 664 915 88         0.000 000 000 49         u
    // neutron molar mass                                          1.008 664 915 88 e-3     0.000 000 000 49 e-3     kg mol^-1
    // neutron-muon mass ratio                                     8.892 484 08             0.000 000 20
    // neutron-proton mag. mom. ratio                              -0.684 979 34            0.000 000 16
    // neutron-proton mass difference                              2.305 573 77 e-30        0.000 000 85 e-30
    // neutron-proton mass difference energy equivalent            2.072 146 37 e-13        0.000 000 76 e-13
    // neutron-proton mass difference energy equivalent in MeV     1.293 332 05             0.000 000 48
    // neutron-proton mass difference in u                         0.001 388 449 00         0.000 000 000 51
    // neutron-proton mass ratio                                   1.001 378 418 98         0.000 000 000 51
    // neutron-tau mass ratio                                      0.528 790                0.000 048
    // neutron to shielded proton mag. mom. ratio                  -0.684 996 94            0.000 000 16
    // Newtonian constant of gravitation over h-bar c              6.708 61 e-39            0.000 31 e-39            (GeV/c^2)^-2
    // nuclear magneton in eV/T                                    3.152 451 2550 e-8       0.000 000 0015 e-8       eV T^-1
    // nuclear magneton in inverse meters per tesla                2.542 623 432 e-2        0.000 000 016 e-2        m^-1 T^-1
    // nuclear magneton in K/T                                     3.658 2690 e-4           0.000 0021 e-4           K T^-1
    // nuclear magneton in MHz/T                                   7.622 593 285            0.000 000 047            MHz T^-1
    // Planck constant in eV s                                     4.135 667 662 e-15       0.000 000 025 e-15       eV s
    // Planck constant over 2 pi in eV s                           6.582 119 514 e-16       0.000 000 040 e-16       eV s
    // Planck constant over 2 pi times c in MeV fm                 197.326 9788             0.000 0012               MeV fm
    // Planck length                                               1.616 229 e-35           0.000 038 e-35           m
    // Planck mass                                                 2.176 470 e-8            0.000 051 e-8            kg
    // Planck mass energy equivalent in GeV                        1.220 910 e19            0.000 029 e19            GeV
    // Planck temperature                                          1.416 808 e32            0.000 033 e32            K
    // Planck time                                                 5.391 16 e-44            0.000 13 e-44            s
    // proton charge to mass quotient                              9.578 833 226 e7         0.000 000 059 e7         C kg^-1
    // proton Compton wavelength                                   1.321 409 853 96 e-15    0.000 000 000 61 e-15    m
    // proton Compton wavelength over 2 pi                         0.210 308 910 109 e-15   0.000 000 000 097 e-15   m
    // proton-electron mass ratio                                  1836.152 673 89          0.000 000 17
    // proton g factor                                             5.585 694 702            0.000 000 017
    // proton gyromag. ratio                                       2.675 221 900 e8         0.000 000 018 e8         s^-1 T^-1
    // proton gyromag. ratio over 2 pi                             42.577 478 92            0.000 000 29             MHz T^-1
    // proton mag. mom.                                            1.410 606 7873 e-26      0.000 000 0097 e-26      J T^-1
    // proton mag. mom. to Bohr magneton ratio                     1.521 032 2053 e-3       0.000 000 0046 e-3
    // proton mag. mom. to nuclear magneton ratio                  2.792 847 3508           0.000 000 0085
    // proton mag. shielding correction                            25.691 e-6               0.011 e-6
    // proton mass energy equivalent                               1.503 277 593 e-10       0.000 000 018 e-10       J
    // proton mass energy equivalent in MeV                        938.272 0813             0.000 0058               MeV
    // proton mass in u                                            1.007 276 466 879        0.000 000 000 091        u
    // proton molar mass                                           1.007 276 466 879 e-3    0.000 000 000 091 e-3    kg mol^-1
    // proton-muon mass ratio                                      8.880 243 38             0.000 000 20
    // proton-neutron mag. mom. ratio                              -1.459 898 05            0.000 000 34
    // proton-neutron mass ratio                                   0.998 623 478 44         0.000 000 000 51
    // proton rms charge radius                                    0.8751 e-15              0.0061 e-15              m
    // proton-tau mass ratio                                       0.528 063                0.000 048
    // Rydberg constant times c in Hz                              3.289 841 960 355 e15    0.000 000 000 019 e15    Hz
    // Rydberg constant times hc in eV                             13.605 693 009           0.000 000 084            eV
    // Rydberg constant times hc in J                              2.179 872 325 e-18       0.000 000 027 e-18       J
    // Sackur-Tetrode constant (1 K, 100 kPa)                      -1.151 7084              0.000 0014
    // Sackur-Tetrode constant (1 K, 101.325 kPa)                  -1.164 8714              0.000 0014
    // shielded helion gyromag. ratio                              2.037 894 585 e8         0.000 000 027 e8         s^-1 T^-1
    // shielded helion gyromag. ratio over 2 pi                    32.434 099 66            0.000 000 43             MHz T^-1
    // shielded helion mag. mom.                                   -1.074 553 080 e-26      0.000 000 014 e-26       J T^-1
    // shielded helion mag. mom. to Bohr magneton ratio            -1.158 671 471 e-3       0.000 000 014 e-3
    // shielded helion mag. mom. to nuclear magneton ratio         -2.127 497 720           0.000 000 025
    // shielded helion to proton mag. mom. ratio                   -0.761 766 5603          0.000 000 0092
    // shielded helion to shielded proton mag. mom. ratio          -0.761 786 1313          0.000 000 0033
    // shielded proton gyromag. ratio                              2.675 153 171 e8         0.000 000 033 e8         s^-1 T^-1
    // shielded proton gyromag. ratio over 2 pi                    42.576 385 07            0.000 000 53             MHz T^-1
    // shielded proton mag. mom.                                   1.410 570 547 e-26       0.000 000 018 e-26       J T^-1
    // shielded proton mag. mom. to Bohr magneton ratio            1.520 993 128 e-3        0.000 000 017 e-3
    // shielded proton mag. mom. to nuclear magneton ratio         2.792 775 600            0.000 000 030
    // standard atmosphere                                         101 325                  (exact)                  Pa
    // standard-state pressure                                     100 000                  (exact)                  Pa
    // tau Compton wavelength                                      0.697 787 e-15           0.000 063 e-15           m
    // tau Compton wavelength over 2 pi                            0.111 056 e-15           0.000 010 e-15           m
    // tau-electron mass ratio                                     3477.15                  0.31
    // tau mass energy equivalent                                  2.846 78 e-10            0.000 26 e-10            J
    // tau mass in u                                               1.907 49                 0.000 17                 u
    // tau molar mass                                              1.907 49 e-3             0.000 17 e-3             kg mol^-1
    // tau-muon mass ratio                                         16.8167                  0.0015
    // tau-neutron mass ratio                                      1.891 11                 0.000 17
    // tau-proton mass ratio                                       1.893 72                 0.000 17
    // triton-electron mass ratio                                  5496.921 535 88          0.000 000 26
    // triton g factor                                             5.957 924 920            0.000 000 028
    // triton mag. mom.                                            1.504 609 503 e-26       0.000 000 012 e-26       J T^-1
    // triton mag. mom. to Bohr magneton ratio                     1.622 393 6616 e-3       0.000 000 0076 e-3
    // triton mag. mom. to nuclear magneton ratio                  2.978 962 460            0.000 000 014
    // triton mass                                                 5.007 356 665 e-27       0.000 000 062 e-27       kg
    // triton mass energy equivalent                               4.500 387 735 e-10       0.000 000 055 e-10       J
    // triton mass energy equivalent in MeV                        2808.921 112             0.000 017                MeV
    // triton mass in u                                            3.015 500 716 32         0.000 000 000 11         u
    // triton molar mass                                           3.015 500 716 32 e-3     0.000 000 000 11 e-3     kg mol^-1
    // triton-proton mass ratio                                    2.993 717 033 48         0.000 000 000 22
    // unified atomic mass unit                                    1.660 539 040 e-27       0.000 000 020 e-27       kg
    // Wien frequency displacement law constant                    5.878 9238 e10           0.000 0034 e10           Hz K^-1
    // Wien wavelength displacement law constant                   2.897 7729 e-3           0.000 0017 e-3           m K

    // Universal.
    PUSH_CONSTANT_NO_UNIT("π (Archimedes' constant pi)", "3.1415926535897932");
    PUSH_CONSTANT_NO_UNIT("e (Euler's number)", "2.7182818284590452");
    PUSH_CONSTANT_NO_UNIT("φ (golden ratio)", "1.6180339887498948");

    // General Physics.
    PUSH_CONSTANT_CODATA("Characteristic Impedance of Vacuum", "376.730313461", "Ω",          "exact",           "2016-03-28");
    PUSH_CONSTANT_CODATA("Dirac's Constant",          "1.054571800e-34",        "J·s",        "0.000000013e-34", "2016-03-28");  // CODATA name: Planck constant over 2 pi
    PUSH_CONSTANT_CODATA("Electric Constant",         "8.854187817e-12",        "F/m",        "exact",           "2016-03-28");
    PUSH_CONSTANT_CODATA("Gravitation Constant",      "6.67408e-11",            "m³/(kg·s²)", "0.00031e-11",     "2016-03-28");
    PUSH_CONSTANT_CODATA("Magnetic Constant",         "12.566370614e-7",        "N/A²",       "exact",           "2016-03-28");  // TODO: Put more figures !
    PUSH_CONSTANT_CODATA("Planck's Constant",         "6.626070040e-34",        "J·s",        "0.000000081e-34", "2016-03-28");
    PUSH_CONSTANT_CODATA("Speed of Light in Vacuum",  "299792458",              "m/s",        "exact",           "2016-03-28");
    PUSH_CONSTANT_CODATA("Standard Gravity",          "9.80665",                "m/s²",       "exact",           "2016-03-28");

    // Electromagnetic.
    PUSH_CONSTANT_CODATA("Bohr-Procopiu Magneton",                      "927.4009994e-26",     "J/T",    "0.0000057e-26",   "2016-03-28");
    PUSH_CONSTANT_CODATA("Conductance Quantum",                         "7.7480917310e-5",     "S",      "0.0000000018e-5", "2016-03-28");
    PUSH_CONSTANT("Coulomb's Constant",                                 "8.987742438e9",       "N·m²/C²");
    PUSH_CONSTANT_CODATA("Elementary Charge",                           "1.602 176 6208 e-19", "C",      "0.0000000098e-19", "2016-03-28");
    PUSH_CONSTANT_CODATA("Conventional value of Josephson Constant",    "483597.9e9",          "Hz/V",   "exact",            "2016-03-28");
    PUSH_CONSTANT_CODATA("Josephson Constant",                          "483597.8525e9",       "Hz/V",   "0.0030e9",         "2016-03-28");
    PUSH_CONSTANT_CODATA("Magnetic Flux Quantum",                       "2.06783383e-15",      "Wb",     "0.000000013e-15",  "2016-03-28");
    PUSH_CONSTANT_CODATA("Nuclear Magneton",                            "5.050783699e-27",     "J/T",    "0.000000031e-27",  "2016-03-28");
    PUSH_CONSTANT_CODATA("Resistance Quantum",                          "12906.4037278",       "Ω",      "0.0000029",        "2016-03-28");  // CODATA name: inverse of conductance quantum
    PUSH_CONSTANT_CODATA("Conventional value of von Klitzing Constant", "25812.807",           "Ω",      "exact",            "2016-03-28");
    PUSH_CONSTANT_CODATA("von Klitzing Constant",                       "25812.8074555",       "Ω",      "0.0000059",        "2016-03-28");

    // Atomic & Nuclear.
    PUSH_CONSTANT_CODATA("Bohr Radius",                    "0.52917721067e-10", "m",     "0.00000000012e-10", "2016-03-28");
    PUSH_CONSTANT_CODATA("Fermi Coupling Constant",        "1.1663787e-5",      "Ge/V²", "0.0000006e-5",      "2016-03-28");
    PUSH_CONSTANT_CODATA("Fine-structure Constant",        "7.2973525664e-3",   "",      "0.0000000017e-3",   "2016-03-28");
    PUSH_CONSTANT_CODATA("Hartree Energy",                 "4.359744650e-18",   "J",     "0.000000054e-18",   "2016-03-28");
    PUSH_CONSTANT_CODATA("Hartree Energy in eV",           "27.21138602",       "eV",    "0.00000017",        "2016-03-28");
    PUSH_CONSTANT_CODATA("Quantum of Circulation",         "3.6369475486e-4",   "m²/s",  "0.0000000017e-4",   "2016-03-28");
    PUSH_CONSTANT_CODATA("Quantum of Circulation times 2", "7.2738950972e-4",   "m²/s",  "0.0000000033e-4",   "2016-03-28");  // I don't know why it is useful, but it is present in CODATA constants, so I added it  -- Hadrien Theveneau, 2016-03-28
    PUSH_CONSTANT_CODATA("Rydberg Constant",               "10973731.568508",   "1/m",   "0.000065",          "2016-03-28");
    PUSH_CONSTANT_CODATA("Thomson Cross Section",          "0.66524587158e-28", "m²",    "0.00000000091e-28", "2016-03-28");
    PUSH_CONSTANT_CODATA("Weak Mixing Angle",              "0.2223",            "",      "0.0021",            "2016-03-28");

    // Physico-chemical.
    PUSH_CONSTANT_CODATA("Atomic Mass Unit",                               "1.660539040e-27",  "kg",         "0.000000020e-27",  "2016-03-28");  // CODATA name: atomic mass constant
    PUSH_CONSTANT_CODATA("Avogadro's Number",                              "6.022140857e23",   "1/mol",      "0.000000074e23",   "2016-03-28");  // CODATA name: Avogadro constant
    PUSH_CONSTANT_CODATA("Boltzmann Constant",                             "1.38064852e-23",   "J/K",        "0.00000079e-23",   "2016-03-28");
    PUSH_CONSTANT_CODATA("Compton wavelength",                             "2.4263102367e-12", "m",          "0.0000000011e-12", "2016-03-28");
    PUSH_CONSTANT_CODATA("Compton wavelength over 2 pi",                   "386.15926764e-15", "m",          "0.00000018e-15",   "2016-03-28");
    PUSH_CONSTANT_CODATA("Electron-volt",                                  "1.6021766208e-19", "J",          "0.0000000098e-19", "2016-03-28");
    PUSH_CONSTANT_CODATA("Faraday Constant",                               "96485.33289",      "C/mol",      "0.00059",          "2016-03-28");
    // PUSH_CONSTANT_CODATA("Faraday constant for conventional electric current", "96 485.3251", "C_90_mol^-1", "0.0012", "2016-03-28");       // TODO: Handle C_90_mol
    PUSH_CONSTANT_CODATA("First Radiation Constant",                       "3.741771790e-16",  "W·m²",       "0.000000046e-16",  "2016-03-28");
    PUSH_CONSTANT_CODATA("First Radiation Constant for Spectral Radiance", "1.191042953e-16",  "(W·m²)/sr",  "0.000000015e-16",  "2016-03-28");
    PUSH_CONSTANT_CODATA("Gas Constant",                                   "8.3144598",        "J/(K·mol)",  "0.0000048",        "2016-03-28");
    PUSH_CONSTANT_CODATA("Loschmidt constant (273.15 K, 100 kPa)",         "2.6516467e25",     "1/m³",       "0.0000015e25",     "2016-03-28");
    PUSH_CONSTANT_CODATA("Loschmidt constant (273.15 K, 101.325 kPa)",     "2.6867811e25",     "1/m³",       "0.0000015e25",     "2016-03-28");
    PUSH_CONSTANT_CODATA("Molar Planck Constant",                          "3.9903127110e-10", "J·s/mol",    "0.0000000018e-10", "2016-03-28");
    PUSH_CONSTANT_CODATA("Second Radiation Constant",                      "1.43877736e-2",    "m·K",        "0.00000083e-2",    "2016-03-28");
    PUSH_CONSTANT_CODATA("Stefan-Boltzmann Constant",                      "5.670367e-8",      "W/(m²·K⁴)",  "0.000013e-8",      "2016-03-28");
    PUSH_CONSTANT_CODATA("{220} Lattice Spacing of Silicon",               "192.0155714e-12",  "m",          "0.0000032e-12",    "2016-03-28");

    // Astronomy.
    PUSH_CONSTANT("Astronomical Unit",      "149597870691",       "m");
    PUSH_CONSTANT("Light Year",             "9.4607304725808e15", "m");
    PUSH_CONSTANT("Parsec",                 "3.08567802e16",      "m");
    PUSH_CONSTANT_NO_UNIT("Gregorian Year", "365.2425");
    PUSH_CONSTANT_NO_UNIT("Julian Year",    "365.25");
    PUSH_CONSTANT_NO_UNIT("Sidereal Year",  "365.2564");
    PUSH_CONSTANT_NO_UNIT("Tropical Year",  "365.2422");
    PUSH_CONSTANT("Earth Mass",             "5.9736e24", "kg");
    PUSH_CONSTANT("Mean Earth Radius",      "6371000",   "m");
    PUSH_CONSTANT("Sun Mass",               "1.9891e30", "kg");
    PUSH_CONSTANT("Sun Radius",             "6.96265e8", "m");
    PUSH_CONSTANT("Sun Luminosity",         "3.827e26",  "W");

    // Molar Masses
    PUSH_CONSTANT("Aluminium",    "26.9815386",  "g/mol");
    PUSH_CONSTANT("Antimony",     "121.760",     "g/mol");
    PUSH_CONSTANT("Argon",        "39.948",      "g/mol");
    PUSH_CONSTANT("Arsenic",      "74.92160",    "g/mol");
    PUSH_CONSTANT("Barium",       "137.327",     "g/mol");
    PUSH_CONSTANT("Beryllium",    "9.012182",    "g/mol");
    PUSH_CONSTANT("Bismuth",      "208.98040",   "g/mol");
    PUSH_CONSTANT("Boron",        "10.811",      "g/mol");
    PUSH_CONSTANT("Bromine",      "79.904",      "g/mol");
    PUSH_CONSTANT("Cadmium",      "112.411",     "g/mol");
    PUSH_CONSTANT("Caesium",      "132.9054519", "g/mol");
    PUSH_CONSTANT("Calcium",      "40.078",      "g/mol");
    PUSH_CONSTANT("Carbon",       "12.0107",     "g/mol");
    PUSH_CONSTANT("Cerium",       "140.116",     "g/mol");
    PUSH_CONSTANT("Chlorine",     "35.453",      "g/mol");
    PUSH_CONSTANT("Chromium",     "51.9961",     "g/mol");
    PUSH_CONSTANT("Cobalt",       "58.933195",   "g/mol");
    PUSH_CONSTANT("Copper",       "63.546",      "g/mol");
    PUSH_CONSTANT("Dysprosium",   "162.500",     "g/mol");
    PUSH_CONSTANT("Erbium",       "167.259",     "g/mol");
    PUSH_CONSTANT("Europium",     "151.964",     "g/mol");
    PUSH_CONSTANT("Fluorine",     "18.9984032",  "g/mol");
    PUSH_CONSTANT("Gadolinium",   "157.25",      "g/mol");
    PUSH_CONSTANT("Gallium",      "69.723",      "g/mol");
    PUSH_CONSTANT("Germanium",    "72.64",       "g/mol");
    PUSH_CONSTANT("Gold",         "196.966569",  "g/mol");
    PUSH_CONSTANT("Hafnium",      "178.49",      "g/mol");
    PUSH_CONSTANT("Helium",       "4.002602",    "g/mol");
    PUSH_CONSTANT("Holmium",      "164.93032",   "g/mol");
    PUSH_CONSTANT("Hydrogen",     "1.00794",     "g/mol");
    PUSH_CONSTANT("Indium",       "114.818",     "g/mol");
    PUSH_CONSTANT("Iodine",       "126.90447",   "g/mol");
    PUSH_CONSTANT("Iridium",      "192.217",     "g/mol");
    PUSH_CONSTANT("Iron",         "55.845",      "g/mol");
    PUSH_CONSTANT("Krypton",      "83.798",      "g/mol");
    PUSH_CONSTANT("Lanthanum",    "138.90547",   "g/mol");
    PUSH_CONSTANT("Lead",         "207.2",       "g/mol");
    PUSH_CONSTANT("Lithium",      "6.941",       "g/mol");
    PUSH_CONSTANT("Lutetium",     "174.9668",    "g/mol");
    PUSH_CONSTANT("Magnesium",    "24.3050",     "g/mol");
    PUSH_CONSTANT("Manganese",    "54.938045",   "g/mol");
    PUSH_CONSTANT("Mercury",      "200.59",      "g/mol");
    PUSH_CONSTANT("Molybdenum",   "95.96",       "g/mol");
    PUSH_CONSTANT("Neodymium",    "144.242",     "g/mol");
    PUSH_CONSTANT("Neon",         "20.1797",     "g/mol");
    PUSH_CONSTANT("Nickel",       "58.6934",     "g/mol");
    PUSH_CONSTANT("Niobium",      "92.90638",    "g/mol");
    PUSH_CONSTANT("Nitrogen",     "14.0067",     "g/mol");
    PUSH_CONSTANT("Osmium",       "190.23",      "g/mol");
    PUSH_CONSTANT("Oxygen",       "15.9994",     "g/mol");
    PUSH_CONSTANT("Palladium",    "106.42",      "g/mol");
    PUSH_CONSTANT("Phosphorus",   "30.973762",   "g/mol");
    PUSH_CONSTANT("Platinum",     "192.084",     "g/mol");
    PUSH_CONSTANT("Potassium",    "39.0983",     "g/mol");
    PUSH_CONSTANT("Praseodymium", "140.90765",   "g/mol");
    PUSH_CONSTANT("Protactinium", "231.03588",   "g/mol");
    PUSH_CONSTANT("Rhenium",      "186.207",     "g/mol");
    PUSH_CONSTANT("Rubidium",     "85.4678",     "g/mol");
    PUSH_CONSTANT("Ruthenium",    "101.07",      "g/mol");
    PUSH_CONSTANT("Samarium",     "150.36",      "g/mol");
    PUSH_CONSTANT("Scandium",     "44.955912",   "g/mol");
    PUSH_CONSTANT("Selenium",     "78.96",       "g/mol");
    PUSH_CONSTANT("Silicon",      "28.0855",     "g/mol");
    PUSH_CONSTANT("Silver",       "107.8682",    "g/mol");
    PUSH_CONSTANT("Sodium",       "22.98976928", "g/mol");
    PUSH_CONSTANT("Strontium",    "87.62",       "g/mol");
    PUSH_CONSTANT("Sulfur",       "32.065",      "g/mol");
    PUSH_CONSTANT("Tantalum",     "180.94788",   "g/mol");
    PUSH_CONSTANT("Tellurium",    "127.60",      "g/mol");
    PUSH_CONSTANT("Terbium",      "158.92535",   "g/mol");
    PUSH_CONSTANT("Thallium",     "204.3833",    "g/mol");
    PUSH_CONSTANT("Thorium",      "232.03806",   "g/mol");
    PUSH_CONSTANT("Thulium",      "168.93421",   "g/mol");
    PUSH_CONSTANT("Tin",          "118.710",     "g/mol");
    PUSH_CONSTANT("Titanium",     "47.867",      "g/mol");
    PUSH_CONSTANT("Tungsten",     "183.84",      "g/mol");
    PUSH_CONSTANT("Uranium",      "238.02891",   "g/mol");
    PUSH_CONSTANT("Vanadium",     "51.9961",     "g/mol");
    PUSH_CONSTANT("Xenon",        "131.293",     "g/mol");
    PUSH_CONSTANT("Ytterbium",    "173.054",     "g/mol");
    PUSH_CONSTANT("Yttrium",      "88.90585",    "g/mol");
    PUSH_CONSTANT("Zinc",         "65.38",       "g/mol");
    PUSH_CONSTANT("Zirconium",    "91.224",      "g/mol");

    // Particle Masses
    // Leptons
    PUSH_CONSTANT_CODATA("Electron Mass", "0.5109989461", "MeV/c²", "0.0000000031", "2016-03-28");
    PUSH_CONSTANT_CODATA("Muon Mass",     "105.6583745",  "MeV/c²", "0.0000024",    "2016-03-28");
    PUSH_CONSTANT_CODATA("Tau Mass",      "1776.82",      "MeV/c²", "0.16",         "2016-03-28");

    // Quarks
    PUSH_CONSTANT("Up-Quark Mass",      "2.3",    "MeV/c²");
    PUSH_CONSTANT("Down-Quark Mass",    "4.8",    "MeV/c²");
    PUSH_CONSTANT("Charm-Quark Mass",   "1.275",  "GeV/c²");
    PUSH_CONSTANT("Strange-Quark Mass", "95",     "MeV/c²");
    PUSH_CONSTANT("Top-Quark Mass",     "173.21", "GeV/c²");
    PUSH_CONSTANT("Bottom-Quark Mass",  "4.18",   "GeV/c²");

    // Bosons
    PUSH_CONSTANT("W-Boson Mass",     "80.385" , "GeV/c²");
    PUSH_CONSTANT("Z-Boson Mass",     "91.1876", "GeV/c²");
    PUSH_CONSTANT("Higgs-Boson Mass", "125.7",   "GeV/c²");

    // Hadrons
    PUSH_CONSTANT_CODATA("Proton Mass",  "938.2720813", "MeV/c²", "0.0000058", "2016-03-28");
    PUSH_CONSTANT_CODATA("Neutron Mass", "939.5654133", "MeV/c²", "0.0000058", "2016-03-28");

    // SI-Units
    PUSH_CONSTANT_CODATA("Electron Mass (SI)", "9.10938356e-31",  "kg", "0.00000011e-31",  "2016-03-28");
    PUSH_CONSTANT_CODATA("Proton Mass (SI)",   "1.672621898e-27", "kg", "0.000000021e-27", "2016-03-28");
    PUSH_CONSTANT_CODATA("Neutron Mass (SI)",  "1.674927471e-27", "kg", "0.000000021e-27", "2016-03-28");
}

void Constants::Private::retranslateText()
{
    auto i = list.begin();
    QString cat;

    // http://en.wikipedia.org/wiki/Mathematical_constant
    cat = Constants::tr("Universal");

    I18N_CONSTANT(Constants::tr("Archimedes' constant Pi") + QString::fromUtf8(" (π)"));
    I18N_CONSTANT(Constants::tr("Euler's number") + QString::fromUtf8(" (ℯ)"));
    I18N_CONSTANT(Constants::tr("Golden ratio") + QString::fromUtf8(" (φ)"));

    // http://en.wikipedia.org/wiki/Physical_constant#Table_of_universal_constants
    cat = Constants::tr("General Physics");

    I18N_CONSTANT(Constants::tr("Characteristic Impedance of Vacuum"));
    I18N_CONSTANT(Constants::tr("Dirac's Constant"));
    I18N_CONSTANT(Constants::tr("Electric Constant"));
    I18N_CONSTANT(Constants::tr("Gravitation Constant"));
    I18N_CONSTANT(Constants::tr("Magnetic Constant"));
    I18N_CONSTANT(Constants::tr("Planck's Constant"));
    I18N_CONSTANT(Constants::tr("Speed of Light in Vacuum"));
    I18N_CONSTANT(Constants::tr("Standard Gravity"));


    // http://en.wikipedia.org/wiki/Physical_constant#Table_of_electromagnetic_constants
    cat = Constants::tr("Electromagnetic");

    I18N_CONSTANT(Constants::tr("Bohr-Procopiu Magneton"));
    I18N_CONSTANT(Constants::tr("Conductance Quantum"));
    I18N_CONSTANT(Constants::tr("Coulomb's Constant"));
    I18N_CONSTANT(Constants::tr("Elementary Charge"));
    I18N_CONSTANT(Constants::tr("Conventional value of Josephson Constant"));
    I18N_CONSTANT(Constants::tr("Josephson Constant"));
    I18N_CONSTANT(Constants::tr("Magnetic Flux Quantum"));
    I18N_CONSTANT(Constants::tr("Nuclear Magneton"));
    I18N_CONSTANT(Constants::tr("Resistance Quantum"));
    I18N_CONSTANT(Constants::tr("Conventional value of von Klitzing Constant"));
    I18N_CONSTANT(Constants::tr("von Klitzing Constant"));

    // CODATA
    cat = Constants::tr("Atomic & Nuclear");

    I18N_CONSTANT(Constants::tr("Bohr Radius"));
    I18N_CONSTANT(Constants::tr("Fermi Coupling Constant"));
    I18N_CONSTANT(Constants::tr("Fine-structure Constant"));
    I18N_CONSTANT(Constants::tr("Hartree Energy"));
    I18N_CONSTANT(Constants::tr("Hartree Energy in eV"));
    I18N_CONSTANT(Constants::tr("Quantum of Circulation"));
    I18N_CONSTANT(Constants::tr("Quantum of Circulation times 2"));
    I18N_CONSTANT(Constants::tr("Rydberg Constant"));
    I18N_CONSTANT(Constants::tr("Thomson Cross Section"));
    I18N_CONSTANT(Constants::tr("Weak Mixing Angle"));

    // CODATA
    cat = Constants::tr("Physico-chemical");

    I18N_CONSTANT(Constants::tr("Atomic Mass Unit"));
    I18N_CONSTANT(Constants::tr("Avogadro's Number"));
    I18N_CONSTANT(Constants::tr("Boltzmann Constant"));
    I18N_CONSTANT(Constants::tr("Compton wavelength"));
    I18N_CONSTANT(Constants::tr("Compton wavelength over 2 pi"));
    I18N_CONSTANT(Constants::tr("Electron volt"));
    I18N_CONSTANT(Constants::tr("Faraday Constant"));
    I18N_CONSTANT(Constants::tr("First Radiation Constant"));
    I18N_CONSTANT(Constants::tr("First Radiation Constant for Spectral Radiance"));
    I18N_CONSTANT(Constants::tr("Gas Constant"));
    I18N_CONSTANT(Constants::tr("Loschmidt constant (273.15 K, 100 kPa)"));
    I18N_CONSTANT(Constants::tr("Loschmidt constant (273.15 K, 101.325 kPa)"));
    I18N_CONSTANT(Constants::tr("Molar Planck Constant"));
    I18N_CONSTANT(Constants::tr("Second Radiation Constant"));
    I18N_CONSTANT(Constants::tr("Stefan-Boltzmann Constant"));
    I18N_CONSTANT(Constants::tr("{220} Lattice Spacing of Silicon"));

    // http://www.astronomynotes.com/tables/tablesa.htm
    cat = Constants::tr("Astronomy");

    I18N_CONSTANT(Constants::tr("Astronomical Unit"));
    I18N_CONSTANT(Constants::tr("Light Year"));
    I18N_CONSTANT(Constants::tr("Parsec"));

    const QString days = Constants::tr("days");

    I18N_CONSTANT_DAYS(Constants::tr("Gregorian Year"));
    I18N_CONSTANT_DAYS(Constants::tr("Julian Year"));
    I18N_CONSTANT_DAYS(Constants::tr("Sidereal Year"));
    I18N_CONSTANT_DAYS(Constants::tr("Tropical Year"));

    I18N_CONSTANT(Constants::tr("Earth Mass"));
    I18N_CONSTANT(Constants::tr("Mean Earth Radius"));
    I18N_CONSTANT(Constants::tr("Sun Mass"));
    I18N_CONSTANT(Constants::tr("Sun Radius"));
    I18N_CONSTANT(Constants::tr("Sun Luminosity"));

    // http://www.ptable.com/
    // Only known constants of accuracy more than an integer are included.
    cat = Constants::tr("Molar Mass");

    I18N_CONSTANT(Constants::tr("Aluminium"));
    I18N_CONSTANT(Constants::tr("Antimony"));
    I18N_CONSTANT(Constants::tr("Argon"));
    I18N_CONSTANT(Constants::tr("Arsenic"));
    I18N_CONSTANT(Constants::tr("Barium"));
    I18N_CONSTANT(Constants::tr("Beryllium"));
    I18N_CONSTANT(Constants::tr("Bismuth"));
    I18N_CONSTANT(Constants::tr("Boron"));
    I18N_CONSTANT(Constants::tr("Bromine"));
    I18N_CONSTANT(Constants::tr("Cadmium"));
    I18N_CONSTANT(Constants::tr("Caesium"));
    I18N_CONSTANT(Constants::tr("Calcium"));
    I18N_CONSTANT(Constants::tr("Carbon"));
    I18N_CONSTANT(Constants::tr("Cerium"));
    I18N_CONSTANT(Constants::tr("Chlorine"));
    I18N_CONSTANT(Constants::tr("Chromium"));
    I18N_CONSTANT(Constants::tr("Cobalt"));
    I18N_CONSTANT(Constants::tr("Copper"));
    I18N_CONSTANT(Constants::tr("Dysprosium"));
    I18N_CONSTANT(Constants::tr("Erbium"));
    I18N_CONSTANT(Constants::tr("Europium"));
    I18N_CONSTANT(Constants::tr("Fluorine"));
    I18N_CONSTANT(Constants::tr("Gadolinium"));
    I18N_CONSTANT(Constants::tr("Gallium"));
    I18N_CONSTANT(Constants::tr("Germanium"));
    I18N_CONSTANT(Constants::tr("Gold"));
    I18N_CONSTANT(Constants::tr("Hafnium"));
    I18N_CONSTANT(Constants::tr("Helium"));
    I18N_CONSTANT(Constants::tr("Holmium"));
    I18N_CONSTANT(Constants::tr("Hydrogen"));
    I18N_CONSTANT(Constants::tr("Indium"));
    I18N_CONSTANT(Constants::tr("Iodine"));
    I18N_CONSTANT(Constants::tr("Iridium"));
    I18N_CONSTANT(Constants::tr("Iron"));
    I18N_CONSTANT(Constants::tr("Krypton"));
    I18N_CONSTANT(Constants::tr("Lanthanum"));
    I18N_CONSTANT(Constants::tr("Lead"));
    I18N_CONSTANT(Constants::tr("Lithium"));
    I18N_CONSTANT(Constants::tr("Lutetium"));
    I18N_CONSTANT(Constants::tr("Magnesium"));
    I18N_CONSTANT(Constants::tr("Manganese"));
    I18N_CONSTANT(Constants::tr("Mercury"));
    I18N_CONSTANT(Constants::tr("Molybdenum"));
    I18N_CONSTANT(Constants::tr("Neodymium"));
    I18N_CONSTANT(Constants::tr("Neon"));
    I18N_CONSTANT(Constants::tr("Nickel"));
    I18N_CONSTANT(Constants::tr("Niobium"));
    I18N_CONSTANT(Constants::tr("Nitrogen"));
    I18N_CONSTANT(Constants::tr("Osmium"));
    I18N_CONSTANT(Constants::tr("Oxygen"));
    I18N_CONSTANT(Constants::tr("Palladium"));
    I18N_CONSTANT(Constants::tr("Phosphorus"));
    I18N_CONSTANT(Constants::tr("Platinum"));
    I18N_CONSTANT(Constants::tr("Potassium"));
    I18N_CONSTANT(Constants::tr("Praseodymium"));
    I18N_CONSTANT(Constants::tr("Protactinium"));
    I18N_CONSTANT(Constants::tr("Rhenium"));
    I18N_CONSTANT(Constants::tr("Rubidium"));
    I18N_CONSTANT(Constants::tr("Ruthenium"));
    I18N_CONSTANT(Constants::tr("Samarium"));
    I18N_CONSTANT(Constants::tr("Scandium"));
    I18N_CONSTANT(Constants::tr("Selenium"));
    I18N_CONSTANT(Constants::tr("Silicon"));
    I18N_CONSTANT(Constants::tr("Silver"));
    I18N_CONSTANT(Constants::tr("Sodium"));
    I18N_CONSTANT(Constants::tr("Strontium"));
    I18N_CONSTANT(Constants::tr("Sulfur"));
    I18N_CONSTANT(Constants::tr("Tantalum"));
    I18N_CONSTANT(Constants::tr("Tellurium"));
    I18N_CONSTANT(Constants::tr("Terbium"));
    I18N_CONSTANT(Constants::tr("Thallium"));
    I18N_CONSTANT(Constants::tr("Thorium"));
    I18N_CONSTANT(Constants::tr("Thulium"));
    I18N_CONSTANT(Constants::tr("Tin"));
    I18N_CONSTANT(Constants::tr("Titanium"));
    I18N_CONSTANT(Constants::tr("Tungsten"));
    I18N_CONSTANT(Constants::tr("Uranium"));
    I18N_CONSTANT(Constants::tr("Vanadium"));
    I18N_CONSTANT(Constants::tr("Xenon"));
    I18N_CONSTANT(Constants::tr("Ytterbium"));
    I18N_CONSTANT(Constants::tr("Yttrium"));
    I18N_CONSTANT(Constants::tr("Zinc"));
    I18N_CONSTANT(Constants::tr("Zirconium"));

    // K.A. Olive et al. (Particle Data Group), Chin. Phys. C, 38, 090001 (2014).
    // http://pdg.lbl.gov/index.html
    // CODATA constants
    cat = Constants::tr("Particle Masses");

    I18N_CONSTANT(Constants::tr("Electron Mass"));      // CODATA, 2016-03-28
    I18N_CONSTANT(Constants::tr("Muon Mass"));          // CODATA, 2016-03-28
    I18N_CONSTANT(Constants::tr("Tau Mass"));           // CODATA, 2016-03-28
    I18N_CONSTANT(Constants::tr("Up-Quark Mass"));      // Particle Data Group
    I18N_CONSTANT(Constants::tr("Down-Quark Mass"));    // Particle Data Group
    I18N_CONSTANT(Constants::tr("Charm-Quark Mass"));   // Particle Data Group
    I18N_CONSTANT(Constants::tr("Strange-Quark Mass")); // Particle Data Group
    I18N_CONSTANT(Constants::tr("Top-Quark Mass"));     // Particle Data Group
    I18N_CONSTANT(Constants::tr("Bottom-Quark Mass"));  // Particle Data Group
    I18N_CONSTANT(Constants::tr("W-Boson Mass"));       // Particle Data Group
    I18N_CONSTANT(Constants::tr("Z-Boson Mass"));       // Particle Data Group
    I18N_CONSTANT(Constants::tr("Higgs-Boson Mass"));   // Particle Data Group
    I18N_CONSTANT(Constants::tr("Proton Mass"));        // CODATA, 2016-03-28
    I18N_CONSTANT(Constants::tr("Neutron Mass"));       // CODATA, 2016-03-28
    I18N_CONSTANT(Constants::tr("Electron Mass (SI)")); // CODATA, 2016-03-28
    I18N_CONSTANT(Constants::tr("Proton Mass (SI)"));   // CODATA, 2016-03-28
    I18N_CONSTANT(Constants::tr("Neutron Mass (SI)"));  // CODATA, 2016-03-28

    categories.clear();
    for (int k = 0; k < list.count(); ++k)
        if (!categories.contains(list.at(k).category))
            categories += list.at(k).category;
    categories.sort();
}

Constants* Constants::instance()
{
    if (!s_constantsInstance) {
        s_constantsInstance = new Constants;
        qAddPostRoutine(s_deleteConstants);
    }
    return s_constantsInstance;
}

Constants::Constants()
    : d(new Constants::Private)
{
    setObjectName("Constants");
    d->populate();
    d->retranslateText();
}

const QList<Constant>& Constants::list() const
{
    return d->list;
}

const QStringList& Constants::categories() const
{
    return d->categories;
}

void Constants::retranslateText()
{
    d->retranslateText();
}

Constants::~Constants()
{
}*/

