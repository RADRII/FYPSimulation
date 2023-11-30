plot_crops <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    dates = AllVals$DATE[First:Last];
    TotalFood = AllVals$CROP_TOTAL[First:Last];
  plot(dates,TotalFood,ylim=c(0,max(TotalFood)),pch='.');
    
  lines(dates,TotalFood,pch='.');

    ## points(dates,berries,pch='.',col="red");
    ## lines(dates,berries,pch='.',col="red");

    ## points(dates,beans,pch='.',col="blue");
    ## lines(dates,beans,pch='.',col="blue");

    
}

mean_crops <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    TotalFood = AllVals$CROP_TOTAL[First:Last];
    summary(TotalFood);
}

plot_wipeouts <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    dates = AllVals$DATE[First:Last];
    areas_in_wipeout = AllVals$NUM_AREAS_IN_WIPEOUT[First:Last];
  plot(dates,areas_in_wipeout,pch='.');
    
  lines(dates,areas_in_wipeout,pch='.');

    ## points(dates,berries,pch='.',col="red");
    ## lines(dates,berries,pch='.',col="red");

    ## points(dates,beans,pch='.',col="blue");
    ## lines(dates,beans,pch='.',col="blue");

    
}

plot_deaths_starve <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    dates = AllVals$DATE[First:Last];
    deaths_starve = AllVals$DEATHS_STARVE[First:Last];
  plot(dates,deaths_starve);
    
}


plot_deaths_age <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    dates = AllVals$DATE[First:Last];
    deaths_age = AllVals$DEATHS_AGE[First:Last];
  plot(dates,deaths_age,ylim=c(1,30));
    
}

mean_people <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    summary(AllVals$POP[First:Last]);
}


    
plot_people <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    dates = AllVals$DATE[First:Last];
    people = AllVals$POP[First:Last];
    typeA = AllVals$TYPEA[First:Last];
        typeB = AllVals$TYPEB[First:Last];
  plot(dates,people,ylim=c(0,max(people)),pch='.');
    
  lines(dates,people,pch='.');

   points(dates,typeA,pch='.',col="red");
   lines(dates,typeA,pch='.',col="red");

   points(dates,typeB,pch='.',col="blue");
   lines(dates,typeB,pch='.',col="blue");

    
}

mean_new <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    summary(AllVals$CROP_INCR[First:Last]);
}

plot_new <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    dates = AllVals$DATE[First:Last];
        TotalNew = AllVals$CROP_INCR[First:Last];
  plot(dates,TotalNew,ylim=c(0,max(TotalNew)),pch='.');
    
  lines(dates,TotalNew,pch='.');


    ## points(dates,berries,pch='.',col="red");
    ## lines(dates,berries,pch='.',col="red");

    ## points(dates,beans,pch='.',col="blue");
    ## lines(dates,beans,pch='.',col="blue");

    
}


plot_hometime <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    dates = AllVals$DATE[First:Last];
    hometime = AllVals$HOMETIME_MAX[First:Last];

  plot(dates,hometime,ylim=c(0,max(hometime)),pch='.');
    
  lines(dates,hometime,pch='.');

    
}

plot_hometime_two <- function(FileName,First,Last) {
    AllVals <- read.table(FileName,header=T);
    dates = AllVals$DATE[First:Last];
    hometime_living= AllVals$HOMETIME_MAX_LIVING[First:Last];
    hometime_dead= AllVals$HOMETIME_MAX_DEAD[First:Last];
    vmax = max(c(max(hometime_living),max(hometime_dead)));

  plot(dates,hometime_living,ylim=c(0,vmax),pch='.',col='red');
    
  lines(dates,hometime_living,pch='.',col='red');

    points(dates,hometime_dead,pch='.',col="blue");
    lines(dates,hometime_dead,pch='.',col="blue");
    


    
}

# age-band histogram
# uses pop_snapshots file, containing DATE TYPE AGE EN EATEN)
# hist frm AGE vals restricted to specified Date and Type
# adjust to to do same for EN or EATEN  
plot_snapshot <- function(FileName,Date,Type='B') {
    AllVals <- read.table(FileName,header=T);
#    DayVals = AllVals[which(AllVals$DATE==Date & as.character(AllVals$TYPE)=='A'),];
#    hist(DayVals$AGE);
#    summary(DayVals);
    DayVals = AllVals[which(AllVals$DATE==Date & as.character(AllVals$TYPE)==Type),];
    #hist(DayVals$AGE, seq(0,500,50), main = paste("pop",Date), ylim = c(0,130));
    #hist(DayVals$AGE, seq(0,500,50), main = paste("pop",Date), ylim = c(0,50));
    #hist(DayVals$HOMETIME, seq(0,80,5), main = paste("pop",Date), ylim = c(0,100));
    #hist(DayVals$AGE, seq(0,500,50), main = paste("pop",Date));
    hist(DayVals$EN, seq(0,7,0.5), main = paste("en",Date));
 #   hist(DayVals$EATEN, seq(0,3.5,0.1), main = paste("eat",Date));
    summary(DayVals);
    
}

# uses pop_snapshots file, (containing DATE TYPE AGE EN EATEN)
# for given type prints sequence of means for EN and EATEN running over given dates
check_means <- function(FileName,StartDate,FinishDate,Type='B') {
    AllVals <- read.table(FileName,header=T);
    #    summary(DayVals);
    for(d in c(StartDate:FinishDate)) {
     DayVals = AllVals[which(AllVals$DATE==d & as.character(AllVals$TYPE)==Type),];
     m1 = mean(DayVals$EN);
          m2 = mean(DayVals$EATEN);
     print(c(d,m1,m2));
    }
}


has_extinction <- function(FileName,Type) {
    extinction_date = -1;
    if(Type == 'A') { TypeName = "TYPEA";  }
    if(Type == 'B') { TypeName = "TYPEB"; }
    AllVals <- read.table(FileName,header=T);
    Dates = AllVals$DATE;
    
    PopCounts = AllVals[[TypeName]];
    PopCounts = PopCounts[61:length(PopCounts)];
    Dates = Dates[61:length(PopCounts)];
    if(min(PopCounts) == 0) {
        extinction_index = min(which(PopCounts == 0));
        extinction_date = Dates[extinction_index];
    }
    
    extinction_date;
}

