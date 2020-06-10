/*
 *
 *   Copyright (c) 2001, Carlos E. Vidales. All rights reserved.
 *
 *   This sample program was written and put in the public domain
 *    by Carlos E. Vidales.  The program is provided "as is"
 *    without warranty of any kind, either expressed or implied.
 *   If you choose to use the program within your own products
 *    you do so at your own risk, and assume the responsibility
 *    for servicing, repairing or correcting the program should
 *    it prove defective in any manner.
 *   You may copy and distribute the program's source code in any
 *    medium, provided that you also include in each copy an
 *    appropriate copyright notice and disclaimer of warranty.
 *   You may also modify this program and distribute copies of
 *    it provided that you include prominent notices stating
 *    that you changed the file(s) and the date of any change,
 *    and that you do not charge any royalties or licenses for
 *    its use.
 *
 *
 *   File Name:  calibrate.h
 *
 *
 *   Definition of constants and structures, and declaration of functions
 *    in Calibrate.c
 *
 */

#ifndef CALIBRATE_H_
#define CALIBRATE_H_

#ifdef __cplusplus
 extern "C" {
#endif

/****************************************************/
/*                                                  */
/* Included files                                   */
/*                                                  */
/****************************************************/



/****************************************************/
/*                                                  */
/* Definitions                                      */
/*                                                  */
/****************************************************/

#ifndef         _CALIBRATE_C_
        #define         EXTERN         extern
#else
        #define         EXTERN
#endif



#ifndef         OK
        #define         OK                      0
        #define         NOT_OK             -1
#endif



#define                 INT_32                           long




/****************************************************/
/*                                                  */
/* Structures                                       */
/*                                                  */
/****************************************************/


typedef struct Point {
                        INT_32    x,
                                 y ;
                     } POINT_T ;



typedef struct Matrix {
                                                        /* This arrangement of values facilitates
                                                         *  calculations within getDisplayPoint()
                                                         */
                        INT_32    An,     /* A = An/Divider */
                                 Bn,     /* B = Bn/Divider */
                                 Cn,     /* C = Cn/Divider */
                                 Dn,     /* D = Dn/Divider */
                                 En,     /* E = En/Divider */
                                 Fn,     /* F = Fn/Divider */
                                 Divider ;
                     } MATRIX ;




/****************************************************/
/*                                                  */
/* Function declarations                            */
/*                                                  */
/****************************************************/


EXTERN int setCalibrationMatrix( POINT_T * display,
								 POINT_T * screen,
                                 MATRIX * matrix) ;


EXTERN int getDisplayPoint( POINT_T * display,
							POINT_T * screen,
                            MATRIX * matrix ) ;
                            
#ifdef __cplusplus
}
#endif

#endif  /* CALIBRATE_H_ */
