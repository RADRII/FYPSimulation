#include "Resource.h"
#include "Location.h"
#include "People.h"
#include "Util.h"
#include "Debug.h"
#include "CmdLineVersions.h"
#include <iostream>

//older code relating to visualisation
//not recently used and prob not entirely compatible
#if USE_QT
#include <QApplication>
#include "QtVis/world_show.h"
#endif

extern bool setup_all_resources_and_locations();

extern void set_up_population();



int main(int argc, char **argv) {

  //sim_params.time_stamp = true; // default is false
  sim_params.results_dir = "/tmp/Toss"; // default is .
  //sim_params.results_dir = "../Results_via_cut_down"; // default is .


  if(sim_params.time_stamp) {
    sim_params.output_suffix = time_stamp();
  }

  //sim_params.random_seed_policy = "via_time";
  //sim_params.random_seed_policy = "user_supplied";
  
  /* begin boilerplate stuff */
  
  rand_setup();

  string stats_name = "r_stats";
  open_an_output(r_stats,stats_name);
  r_line.write_header(r_stats);


  open_an_output(setup_record,"sim_setup");



  open_an_output(gains_info_history, "gains_info");
  write_gains_info_header(gains_info_history);

  
  open_an_output(pop_snapshots, "pop_snapshots");
  write_pop_snapshot_header(pop_snapshots);

  open_an_output(starvation_stats,"starvation_stats");
  write_starvation_stats_header(starvation_stats);  

#if DEBUG1
  init_db_file("/tmp/Toss/junk");
  //db_level = 1;
  db_level = 0;
  db_to_file = true;
  //db_to_file = false;
#endif
  
#if USE_QT
  // do bare qt setup
  QApplication app(argc, argv);
#endif

  setup_record << "random_seed_policy was:" << sim_params.random_seed_policy << endl;

  unsigned long int seed;

  if(sim_params.random_seed_policy == "via_default") {}
  else if (sim_params.random_seed_policy == "via_time") {
    // let current time set the seed
    seed = time(NULL);
    gsl_rng_set(r_global, seed);
    setup_record << "time based seed was:" << seed << endl;
  }
  else if (sim_params.random_seed_policy == "user_supplied") {
    gsl_rng_set(r_global, seed);
    setup_record << "supplied seed was:" << seed << endl;
  }
  

#if USE_QT
  //  WorldShow::res_level = -1;
  WorldShow::res_level = 0;
#endif
  
  /* end boilerplate stuff */

  /****************************************************/
  /* SET UP RESOURCES and LOCATIONS                   */
  /* sets up all_home_loc, all_res_entry_loc, all_res */
  /* and  all links                                   */
  /****************************************************/
  setup_all_resources_and_locations();    

  /****************************************/
  /* first 60 days without any people     */
  /* so will just see resources naturally */
  /* going up and down                    */
  /****************************************/
  for(int date=0; date < 60; date++) {

#if DEBUG
    db(date); db(":\n");
#endif

    // UPDATE RESOURCES JUST BY GROWTH
    double incr = 0.0;
    double total = 0.0;
    int num_areas_in_wipeout = 0;
    for(size_t i = 0; i < all_res.size(); i++) {
      incr += all_res[i]->update_at_date(date);
      total += all_res[i]->get_total();
      if(all_res[i]->in_wipeout) { num_areas_in_wipeout++; }
    }

    for(size_t i = 0; i < all_res.size(); i++) {
#if DEBUG
      db(all_res[i]->tostring()); db("\n");
      //db_level = 0;
      //all_res[i]->show_bands(); //db_level = 1;
      //db_level = 0;
      //all_res[i]->show_bars(); //db_level = 1;
      // for brief output
      all_res[i]->show_total();
      db("\n");
#endif
    }
    
    pop.qt_show_crops();

    r_line.DATE = date;
    r_line.CROP_INCR = incr;
    r_line.CROP_TOTAL = total;
    r_line.NUM_AREAS_IN_WIPEOUT = num_areas_in_wipeout;
    r_line.DEATHS_AGE = 0;
    r_line.DEATHS_STARVE = 0;

    r_line.BIRTHS = 0;
    r_line.POP = 0;
    r_line.TYPEA = 0;
    r_line.TYPEB = 0;
    r_line.A_EN = 0;
    r_line.B_EN = 0;
    r_line.A_EATEN = 0;
    r_line.B_EATEN = 0;
    
    r_line.HOMETIME_MAX_LIVING = 0;
    r_line.HOMETIME_MAX_DEAD = 0;
    r_line.HOMETIME_MAX = 0;
    r_line.TWOPLACETIME_MAX = 0;
    r_line.MAX_NUM_PLACES_EATEN = 0;
    r_line.MAX_NUM_PLACES_EXPLORED = 0;
    r_line.write(r_stats);

    /* write dummy lines in gains_info_history for days without people */
    /* date 0 .. 0                                                      */
    gains_info_history << date;
    for(int i = 0; i < count_spaces(gains_info_header); i++) { gains_info_history << " 0"; }
    gains_info_history << "\n";


	
  }

  /***************************************/
  /* SET UP POPULATION                   */
  /* sets pop                            */
  /***************************************/
  set_up_population();
  
#if DEBUG
  pop.show();
#endif

#if USE_QT
  //WorldShow::res_level = -1;
  WorldShow::res_level = 0;
  //WorldShow::res_level = 1;
#endif

  
  int extinction_date = 0;
  char extinct_tribe;

  int date_max = 0;
  //date_max = 1500;
  date_max = 25000;

  for(int date=60; date < date_max; date++) {

    cout << "it: " << date << endl;
    
#if DEBUG1
    //db_level = 1;
    db(date); db(" STARTS\n");
    //db_level = 0;
#endif
   
    r_line.DATE = date;

    // UPDATE RESOURCES BY GROWTH
#if DEBUG
    db("res:\n");
#endif

    double incr = 0.0;
    double total = 0.0;
    int num_areas_in_wipeout = 0;
    for(size_t i = 0; i < all_res.size(); i++) {
      incr += all_res[i]->update_at_date(date);
      total += all_res[i]->get_total();
      if(all_res[i]->in_wipeout) { num_areas_in_wipeout++; }
    }

    
    for(size_t i = 0; i < all_res.size(); i++) {
#if DEBUG
      
      db(all_res[i]->tostring()); 
      if(all_res[i]->in_wipeout) { db(" in wipeout"); }
      db("\n");
      all_res[i]->show_bands();
      // for briefer output
      all_res[i]->show_total();
      db("\n"); 
      // don't really need this as well
      // all_res[i]->show_bars();
      
#endif
    }


    
#if USE_QT
    WorldShow::res_level = -1;
#endif
    
    pop.qt_show_crops();
    //   WorldShow::res_level = -1;
    // skip bars
    for(size_t i = 0; i < all_res.size(); i++) {
#if DEBUG
      //db_level = 0;
      //all_res[i]->show_bars();
      db("\n");
      //db_level = 1;
#endif
    }

    r_line.CROP_INCR = incr;
    r_line.CROP_TOTAL = total;
    r_line.NUM_AREAS_IN_WIPEOUT = num_areas_in_wipeout;

    /**************************************************************/
    /* UPDATE RESOURCES AND PEOPLE BY EATING, DEATH, REPRODUCTION */
    /**************************************************************/
    
    pop.zero_eaten_today();
    pop.clear_all_area_gains();
    
    // TEMP: zero records of num places eaten, num places explored, num speaks
    for(size_t i = 0; i < pop.population.size(); i++) {
      pop.population[i]->clear_places_eaten();
      pop.population[i]->clear_places_explored();
      pop.population[i]->home_time = 0;

    }

#if DEBUG
    db("people:\n");
    pop.show();
    //db_level = 0;
#endif
    
    
#if USE_QT
    WorldShow::res_level = -1;
    //WorldShow::res_level = 1;
#endif

    // this does all the updates
    pop.update(date);
    // side effect is lots updates to r_line
    // eg DEATHS(_AGE,_STARVE,_THREAT), BIRTHS, TYPEA, TYPEB, POP, A_EN, B_EN, A_EATEN, B_EATEN, HOMETIME_MAX  

    write_gains_info_line(gains_info_history);


     /* block to write pop snapshots */
     // if((date >= 60) && (date <= 1000)) {
     //   write_pop_snapshot(pop_snapshots);

     // }

    if(pop.get_total() == 0) {
      
      r_line.write(r_stats);
#if DEBUG
      db("extinction\n");
#endif
      break;
    }

    
    if(((r_line.TYPEA == 0) | (r_line.TYPEB == 0)) && extinction_date == 0) {

      extinction_date = date;
      if((r_line.TYPEA) == 0) {  extinct_tribe = 'A';}
      if((r_line.TYPEB) == 0) {  extinct_tribe = 'B';}
      
      // skip this to let run on
      // date_max = 1.1 * date; // let it continue for 10% of current run
      
      if(r_line.TYPEA == 0) {
    	cout << "A went extinct on day" << date << endl;
      }
      if(r_line.TYPEB == 0) {
    	cout << "B went extinct on day" << date << endl;
      }
    }
    
    r_line.write(r_stats);

  }

  r_stats.close();

  setup_record.close();

  gains_info_history.close();

  
  pop_snapshots.close();

  starvation_stats.close();

#if DEBUG1
  close_db_file();
#endif
  if(extinction_date != 0) {
    cout << extinct_tribe << " --> 0 " << extinction_date << endl;
  }
  cout << "see: " << sim_params.output_suffix << endl;

  return 0;
}

  

