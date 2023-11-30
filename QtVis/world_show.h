#ifndef WORLD_SHOW_H
#define WORLD_SHOW_H

#include <QtGui>
#include "../Resource.h"
#include "../People.h"
#include "../Location.h"
#include "../Util.h"

class WorldShow : public QDialog
{
  Q_OBJECT

 public:
  WorldShow(Population *p);
  int whole_width;
  int whole_height;
  static int res_level; // 0 for day-by-day, 1 for event-by-event
  void plot_resource(ResPtr r, QPainter *painter,  int plot_left,  int write_left, int middle);
  public slots: 
    void run_pic();
    void close_slot(); 
    void pause_slot(); 



 private:

    QLabel *the_label;
    QPushButton *button;
    //    QPushButton *quit_button;
    //   int type;
    // QTimer *timer;
    int time_till_close;
    Population *pop_ptr;
    // this now global
    //    vector<ResPtr> *all_res; 

    bool paused;
    
};


#endif


