/*************************************************************************
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Kohei Yoshida.
 *    1039 Kingsway Dr., Apex, NC 27502, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "numeric/matrix.hxx"
#include <stdio.h>

using namespace ::scsolver::numeric;

void basicIO()
{
    printf("Original empty matrix\n");
    Matrix mx(3, 3);
    mx.print();     // empty matrix
    printf("Simple assignment: (0, 0) = 4.54   (0, 2) = 12.55   (1, 1) = 1.2   (2, 1) = 35.4   (2, 2) = 5\n");
    mx(0, 0) = 4.54;
    mx(0, 2) = 12.55;
    mx(1, 1) = 1.2;
    mx(2, 1) = 35.4;
    mx(2, 2) = 5.0;
    mx.print();

    printf("Transposing matrix\n");
    mx.trans().print();

    // Automatic resizing.
    printf("Automatic resizing\n");
    mx.setResizable(true);
    mx(3, 10) = 100;
    mx.print();
    mx.setResizable(false);

    // Shrink the matrix.
    printf("Shrinking the matrix to 3 x 3\n");
    mx.resize(3, 3);
    mx.print();

    // Expanding the matrix
    printf("Expanding the matrix to 5 x 5\n");
    mx.resize(5, 5);
    mx.print();

    // Shrinking it again.
    printf("Shrinking it again\n");
    mx.resize(3, 3);
    mx.print();

    // Bad index
    mx.setResizable(false);
    try
    {
        printf("Bad index: (-1, 0) = 999\n");
        mx(-1, 0) = 999;
    }
    catch (const BadIndex&)
    {
        printf("BadIndex exception caught on assigning a new value.\n");
    }
    mx.print();

    try
    {
        printf("Bad index: (0, -1) = 999\n");
        mx(0, -1) = 999;
    }
    catch (const BadIndex&)
    {
        printf("BadIndex exception caught on assigning a new value.\n");
    }
    mx.print();

    try
    {
        printf("Bad index: r = (10, 10)\n");
        double r = mx(10, 10);
        r = 20.5;
    }
    catch (const BadIndex&)
    {
        printf("BadIndex exception caught on querying a value.\n");
    }
    mx.print();
    mx.setResizable(true);

    printf("3 x 3 identity matrix\n");
    Matrix mxi(3, 3, true);
    mxi.print();
}

int main()
{
    printf("unit test: Matrix\n");
    basicIO();
    printf("Unit test passed!\n");
}
