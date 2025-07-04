OBJS=obj/main.o obj/content_view.o obj/toolbar.o obj/navbar.o obj/treeview.o obj/sizebar.o obj/statusbar.o obj/file_node.o obj/file_actions.o obj/input_dialog.o obj/resource.o
INCLUDE_DIR=-I.\include -I.\include\libcdio
EXE_NAME=wfm.exe

LDFLAGS=-s -lcomctl32 -lgdi32 -lole32 -luuid .\libcdio.dll -Wl,--subsystem,windows
RC=windres
CFLAGS=-O2 -std=c99 -DUNICODE -D_UNICODE -DCOBJMACROS -DWINVER=0x0600 -Wall

all: ${EXE_NAME}

${EXE_NAME}: ${OBJS}
	${CC} -o ${EXE_NAME} ${OBJS} ${LDFLAGS}

clean:
	del obj\*.o ${EXE_NAME}

obj:
	mkdir obj

obj/%.o: src/%.c obj
	${CC} ${CFLAGS} ${INCLUDE_DIR} -c $< -o $@

obj/resource.o: res/resource.rc res/Application.manifest res/main.ico res/go.ico res/refresh.ico res/search.ico res/nav_arrow.ico res/up.ico res/copy.ico res/cut.ico res/paste.ico res/delete.ico res/new_folder.ico res/new_file.ico include/resource.h
	${RC} ${INCLUDE_DIR} -I.\res -i $< -o $@