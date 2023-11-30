#CPPFLAGS = -g -DDEBUG
#CPPFLAGS = -g -DDEBUG1 -DDEBUG
#CPPFLAGS = -O2 -DDEBUG1 -DDEBUG -DDEBUG2
CPPFLAGS = -O2
#CPPFLAGS = -g

LIBS = -lgsl -lgslcblas -lm 

QtVisDir = QtVis

# use this with USE_QT
#QtVisObjects = $(QtVisDir)/world_show.o $(QtVisDir)/moc_world_show.o

# frm Qt generated Makefile
# also use these with USE_QT
# QTDEFINES       = -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
# #QTCXXFLAGS      = -m64 -pipe -O2 -Wall -W -D_REENTRANT $(QTDEFINES)
# QTCXXFLAGS      = -m64 -pipe -Wall -W -D_REENTRANT $(QTDEFINES)
# QTINCPATH       = -I/usr/share/qt4/mkspecs/linux-g++-64 -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I. -I.
# QTLIBS          = -L/usr/lib/x86_64-linux-gnu -lQtGui -lQtCore -lpthread
# QTLFLAGS        = -m64 -Wl,-O1




DayStats.o: DayStats.cpp DayStats.h
	g++ $(CPPFLAGS) $(INCPATH) -c DayStats.cpp

Util.o: Util.cpp Util.h People.h
	g++ $(CPPFLAGS) $(INCPATH) -c Util.cpp

CmdLineVersions.o: CmdLineVersions.cpp CmdLineVersions.h
	g++ $(CPPFLAGS) $(INCPATH) -c CmdLineVersions.cpp

Resource.o: Resource.cpp Resource.h Location.h Util.h DayStats.h
	g++ $(CPPFLAGS) $(INCPATH) -c Resource.cpp


Debug.o: Debug.cpp Debug.h
	g++ $(CPPFLAGS) $(INCPATH) -c Debug.cpp

Knowledge.o: Knowledge.cpp Knowledge.h InfoTypes.h AreaGain.h Resource.h Location.h Util.h DayStats.h
	g++ $(CPPFLAGS) $(INCPATH) -c Knowledge.cpp

AreaGain.o: AreaGain.cpp AreaGain.h Resource.h
	g++ $(CPPFLAGS) $(INCPATH) -c AreaGain.cpp

InfoTypes.o: InfoTypes.cpp InfoTypes.h AreaGain.h Resource.h
	g++ $(CPPFLAGS) $(INCPATH) -c InfoTypes.cpp

People.o: People.cpp People.h Knowledge.h InfoTypes.h AreaGain.h Resource.h Location.h Util.h DayStats.h Debug.h
	g++ $(CPPFLAGS) $(INCPATH) $(QTCXXFLAGS) $(QTINCPATH) -c People.cpp




People_tester.o: People_tester.cpp People.h Knowledge.h InfoTypes.h AreaGain.h Resource.h Location.h Event.h Util.h DayStats.h Debug.h CmdLineVersions.h $(QtVisDir)/world_show.h
	g++ $(CPPFLAGS) $(INCPATH) $(QTCXXFLAGS) $(QTINCPATH) -c People_tester.cpp

People_tester: People_tester.o world_setup.o People.o Knowledge.o InfoTypes.o AreaGain.o Resource.o Location.o Event.o Util.o DayStats.o Debug.o CmdLineVersions.o $(QtVisObjects)
	g++ $(CPPFLAGS) $(QTCXXFLAGS) People_tester.o world_setup.o People.o Knowledge.o InfoTypes.o AreaGain.o Resource.o Location.o Event.o Util.o DayStats.o Debug.o CmdLineVersions.o $(LIBS) $(QtVisObjects) $(QTLFLAGS) $(QTLIBS) -o People_tester


world_setup.o: world_setup.cpp People.h Resource.h Location.h Event.h Util.h DayStats.h Debug.h
	g++ $(CPPFLAGS) $(INCPATH) -c world_setup.cpp


Event.o: Event.cpp Event.h Location.h People.h Resource.h Debug.h
	g++ $(CPPFLAGS) $(INCPATH) -c Event.cpp


Location.o: Location.cpp Location.h Debug.h
	g++ $(CPPFLAGS) $(INCPATH) -c Location.cpp


clean:
	rm People_tester.o world_setup.o People.o Knowledge.o InfoTypes.o AreaGain.o Resource.o Location.o Event.o Util.o DayStats.o Debug.o CmdLineVersions.o People_tester

