C
C	The Web Viewer
C
C		WV simple server FORTRAN test code
C
C	Copyright 2011-2012, Massachusetts Institute of Technology
C	Licensed under The GNU Lesser General Public License, version 2.1
C	See http://www.opensource.org/licenses/lgpl-2.1.php
C
        include "wsserver.inc"
C
        integer*8 context
        real      eye(3), center(3), up(3), offset(3)
C
        data eye   /0.0, 0.0, 7.0/
        data center/0.0, 0.0, 0.0/
        data up    /0.0, 1.0, 0.0/
C
        call iv_createContext(0, 30.0, 1.0, 10.0, eye, center, up, 
     &                        context)
        if (context .eq. 0) then
          write(*,*) "failed to create wvContext!"
          stop
        endif
C
C       make the scene
        offset(1) =  0.0
        offset(2) =  0.0
        offset(3) =  0.0
        call createBox(context, "Box1", WV_ON+WV_SHADING+WV_ORIENTATION, 
     &                 offset)
C
        offset(1) =  0.1
        offset(2) =  0.1
        offset(3) =  0.1
        call createBox(context, "Box2", WV_ON+WV_TRANSPARENT, offset)
C        
        call createLines(context, "Lines", WV_ON)
C        
        offset(1) = -0.1
        offset(2) = -0.1
        offset(3) = -0.1
        call createPoints(context, "Points", WV_ON, offset)
C
C       start server
        if (iv_startServer(7681, "", "", "", 0, context) .eq. 0) then
 1        if (iv_statusServer(0) .eq. 0) go to 2
C         we have a single valid server -- do nothing:
          call iv_usleep(500000)
          go to 1
        else
          write(*,*) " startServer failed!"
        endif
        
 2      call iv_cleanupServers()
        stop
        end


        subroutine browserMessage(wsi, message)
        integer*8     wsi
        character*(*) message
C
        write(*,*) "from Browser: ", message
C
C       ping it back
        call iv_sendText(wsi, message)
C
        return
        end
        
        
        subroutine createBox(cntxt, name, attr, offset)
C
        integer*8     cntxt
        character*(*) name
        integer       attr
        real          offset(3)
C
        include "wsserver.inc"
C
        integer   i, n, attrs;
        integer*8 items(5);
        real      vertices(72), normals(72)
        integer*1 colors(72)
        integer   indices(36), oIndices(24)

C       box:
C       v6----- v5
C       /|      /|
C       v1------v0|
C       | |     | |
C       | |v7---|-|v4
C       |/      |/
C       v2------v3
C
C       vertex coords array
        data vertices/ 1, 1, 1,  -1, 1, 1,  -1,-1, 1,   1,-1, 1,   
     &                 1, 1, 1,   1,-1, 1,   1,-1,-1,   1, 1,-1, 
     &                 1, 1, 1,   1, 1,-1,  -1, 1,-1,  -1, 1, 1, 
     &                -1, 1, 1,  -1, 1,-1,  -1,-1,-1,  -1,-1, 1, 
     &                -1,-1,-1,   1,-1,-1,   1,-1, 1,  -1,-1, 1, 
     &                 1,-1,-1,  -1,-1,-1,  -1, 1,-1,   1, 1,-1 /
C
C       normal array
        data normals/ 0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 1,
     &                1, 0, 0,   1, 0, 0,   1, 0, 0,   1, 0, 0,
     &                0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,
     &               -1, 0, 0,  -1, 0, 0,  -1, 0, 0,  -1, 0, 0,
     &                0,-1, 0,   0,-1, 0,   0,-1, 0,   0,-1, 0,
     &                0, 0,-1,   0, 0,-1,   0, 0,-1,   0, 0,-1 /
C
C       color array
        data colors/ 0,0,255,    0,0,255,    0,0,255,    0,0,255,
     &               255,0,0,    255,0,0,    255,0,0,    255,0,0,
     &               0,255,0,    0,255,0,    0,255,0,    0,255,0,
     &               255,255,0,  255,255,0,  255,255,0,  255,255,0,
     &               255,0,255,  255,0,255,  255,0,255,  255,0,255,
     &               0,255,255,  0,255,255,  0,255,255,  0,255,255 /
C
C       index array
        data indices/  0, 1, 2,   0, 2, 3,  4, 5, 6,   4, 6, 7,
     &                 8, 9,10,   8,10,11, 12,13,14,  12,14,15,
     &                16,17,18,  16,18,19, 20,21,22,  20,22,23 /
C          
C       other index array
        data oIndices/ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
     &                16,17,18,19,20,21,22,23/
C
        do i = 1, 72, 3
          vertices(i  ) = vertices(i  ) + offset(1)
          vertices(i+1) = vertices(i+1) + offset(2)
          vertices(i+2) = vertices(i+2) + offset(3)
        enddo
C
        i = iv_setData(WV_REAL32, 24, vertices, WV_VERTICES, items(1))
        if (i < 0) 
     &    write(*,*) " wv_setData = ", i, " for ", name, " item 1!"
        i = iv_setData(WV_INT32,  36, indices,  WV_INDICES,  items(2))
        if (i < 0) 
     &    write(*,*) " wv_setData = ", i, " for ", name, " item 2!"
        i = iv_setData(WV_UINT8,  24, colors,   WV_COLORS,   items(3))
        if (i < 0) 
     &    write(*,*) " wv_setData = ", i, " for ", name, " item 3!"
        i = iv_setData(WV_REAL32, 24, normals,  WV_NORMALS,  items(4))
        if (i < 0) 
     &    write(*,*) " wv_setData = ", i, " for ", name, " item 4!"
        n     = 4
        attrs = attr
        if (name .eq. "Box1") then
          i = iv_setData(WV_INT32, 24, oIndices, WV_PINDICES, items(5))
          if (i < 0) 
     &      write(*,*) " wv_setData = ", i, " for ", name, " item 5!"
          n = n + 1
          attrs = attrs + WV_POINTS;
        endif
        if (name .eq. "Box2")  then
          i = iv_setData(WV_INT32, 24, oIndices, WV_LINDICES, items(5))
          if (i < 0) 
     &      write(*,*) " wv_setData = ", i, " for ", name, " item 5!"
          n = n + 1
          attrs = attrs + WV_LINES;
        endif
C
        i = iv_addGPrim(cntxt, name, WV_TRIANGLE, attrs, n, items)
        if (i < 0) write(*,*) " wv_addGPrim = ", i, " for ", name, "!"
C
        return
        end


        subroutine createLines(cntxt, name, attr)
C
        integer*8     cntxt
        character*(*) name
        integer       attr
C
        include "wsserver.inc"
C
        integer   i;
        integer*8 items(2);
        real      vertices(72)
        integer   indices(48)

C       box:
C       v6----- v5
C       /|      /|
C       v1------v0|
C       | |     | |
C       | |v7---|-|v4
C       |/      |/
C       v2------v3
C
C       vertex coords array
        data vertices/ 1, 1, 1,  -1, 1, 1,  -1,-1, 1,   1,-1, 1,   
     &                 1, 1, 1,   1,-1, 1,   1,-1,-1,   1, 1,-1, 
     &                 1, 1, 1,   1, 1,-1,  -1, 1,-1,  -1, 1, 1, 
     &                -1, 1, 1,  -1, 1,-1,  -1,-1,-1,  -1,-1, 1, 
     &                -1,-1,-1,   1,-1,-1,   1,-1, 1,  -1,-1, 1, 
     &                 1,-1,-1,  -1,-1,-1,  -1, 1,-1,   1, 1,-1 /
C
C       index array
        data indices/ 0, 1,  1, 2,  2, 3,  3, 0,
     &                4, 5,  5, 6,  6, 7,  7, 4, 
     &                8, 9,  9,10, 10,11, 11, 8,
     &               12,13, 13,14, 14,15, 15,12,
     &               16,17, 17,18, 18,19, 19,16,
     &               20,21, 21,22, 22,23, 23,20 /
C
        i = iv_setData(WV_REAL32, 24, vertices, WV_VERTICES, items(1))
        if (i < 0) 
     &    write(*,*) " wv_setData = ", i, " for ", name, " item 1!"
        i = iv_setData(WV_INT32,  48, indices, WV_INDICES,   items(2))
        if (i < 0) 
     &    write(*,*) " wv_setData = ", i, " for ", name, " item 2!"
C
        i = iv_addGPrim(cntxt, name, WV_LINE, attrs, 2, items)
        if (i < 0) write(*,*) " wv_addGPrim = ", i, " for ", name, "!"
C
        return
        end


        subroutine createPoints(cntxt, name, attr, offset)
C
        integer*8     cntxt
        character*(*) name
        integer       attr
        real          offset(3)
C
        include "wsserver.inc"
C
        integer   i;
        integer*8 items(2);
        real      colors(3), vertices(72)
C
        data colors/0.6, 0.6, 0.6/
C
C       box:
C       v6----- v5
C       /|      /|
C       v1------v0|
C       | |     | |
C       | |v7---|-|v4
C       |/      |/
C       v2------v3
C
C       vertex coords array
        data vertices/ 1, 1, 1,  -1, 1, 1,  -1,-1, 1,   1,-1, 1,   
     &                 1, 1, 1,   1,-1, 1,   1,-1,-1,   1, 1,-1, 
     &                 1, 1, 1,   1, 1,-1,  -1, 1,-1,  -1, 1, 1, 
     &                -1, 1, 1,  -1, 1,-1,  -1,-1,-1,  -1,-1, 1, 
     &                -1,-1,-1,   1,-1,-1,   1,-1, 1,  -1,-1, 1, 
     &                 1,-1,-1,  -1,-1,-1,  -1, 1,-1,   1, 1,-1 /
C      
        do i = 1, 72, 3
          vertices(i  ) = vertices(i  ) + offset(1)
          vertices(i+1) = vertices(i+1) + offset(2)
          vertices(i+2) = vertices(i+2) + offset(3)
        enddo
C
        i = iv_setData(WV_REAL32, 24, vertices, WV_VERTICES, items(1))
        if (i < 0) 
     &    write(*,*) " wv_setData = ", i, " for ", name, " item 1!"
        i = iv_setData(WV_REAL32,  1, colors, WV_COLORS,     items(2))
        if (i < 0) 
     &    write(*,*) " wv_setData = ", i, " for ", name, " item 2!"
C
        i = iv_addGPrim(cntxt, name, WV_POINT, attr, 2, items)
        if (i < 0) write(*,*) " wv_addGPrim = ", i, " for ", name, "!"
C
        return
        end
