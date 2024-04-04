// Enter Spreadsheet ID here
var SS = SpreadsheetApp.openById('1JDhYswStR_TUifs0UsQ2OwBKfMNsKKNInYb0ByWr0Z8');
var mainTankLog = SS.getSheetByName('mainTankLog'); // sheet name -> mainTankLog
var div1TankLog = SS.getSheetByName('div1TankLog'); // sheet name -> div1TankLog
var appLog = SS.getSheetByName('appLog'); // sheet name -> appLog
var timezone = "asia/delhi"
var Curr_Date;
var Curr_Time;

function doPost(e) {
  Curr_Date = Utilities.formatDate(new Date(), "GMT+5:30", "yyyy/MM/dd"); //gets the current date
  Curr_Time = Utilities.formatDate(new Date(), "GMT+5:30", "hh:mm:ss a"); // gets the current time
  appLog.insertRows(2);
  appLog.getRange('A2').setValue(Curr_Date);
  appLog.getRange('B2').setValue(Curr_Time);
  appLog.getRange('C2').setValue(e);
  var result = h2o_app(e);
  appLog.getRange('D2').setValue(result);
  sendToMongoose();
  SpreadsheetApp.flush();
  return ContentService.createTextOutput(result);
}

function h2o_app(e) {
  var parsedData;
  try { 
    parsedData = JSON.parse(e.postData.contents);
  } 
  catch (err){
    return "Error in parsing request body: " + err.message;
  }
  if (parsedData !== undefined || parsedData.format === undefined){
    switch (parsedData.command) {
      case "main_tank":
        return main_tank(parsedData.values);
      case "div_1_tank":
        return div_1_tank(parsedData.values);
      default :
        return "Unknown command" + parsedData;
    }
  } // endif (parsedData !== undefined)
  else {
    return "Error! Request body empty or in incorrect format.";
  }
}

function main_tank(values) {
  var dataArr = values.split(",");
  var val_1 = dataArr [0]; // val_1 from Arduino code - tank level
  var val_2 = dataArr [1]; // val_2 from Arduino code - tank Liters
  var val_3 = dataArr [2]; // val_3 from Arduino code - div1 on time
  var val_4 = dataArr [3]; // val_4 from Arduino code - div1 off time
  // var val_5 = dataArr [4]; // val_5 from Arduino code - div1 valve auto on time
  // var val_6 = dataArr [5]; // val_6 from Arduino code - div1 valve auto on time duration

  var date_A2 = mainTankLog.getRange('A2').getValue();
  var past_Date = Utilities.formatDate(date_A2, "GMT+5:30", "yyyy/MM/dd");
  if ( past_Date.toString().match(Curr_Date.toString()) ) {
    var preLit = parseInt( mainTankLog.getRange('D2').getValue() );
    var ci = calcConsInta(preLit, val_2).split(",");
    var con = parseInt(ci[0]) + parseInt( mainTankLog.getRange('F2').getValue() );
    var inta = parseInt(ci[1]) + parseInt( mainTankLog.getRange('H2').getValue() );
    mainTankLog.getRange('B2').setValue(Curr_Time);
    mainTankLog.getRange('C2').setValue(val_1);  
    mainTankLog.getRange('D2').setValue(val_2);
    mainTankLog.getRange('F2').setValue(con);
    mainTankLog.getRange('G2').setValue(con);
    mainTankLog.getRange('H2').setValue(inta);
  }
  else {
    var sum = 0;
    for (var i = 3 ; i < 3+7 ; i++){
      var val = mainTankLog.getRange(i, 6).getValues();
      sum = sum + parseInt(val);
    }
    var avg_con = sum / 7;
    mainTankLog.insertRows(2); // insert full row directly below header text
    mainTankLog.getRange('A2').setValue(Curr_Date);
    mainTankLog.getRange('B2').setValue(Curr_Time);
    mainTankLog.getRange('C2').setValue(val_1);
    mainTankLog.getRange('D2').setValue(val_2);
    mainTankLog.getRange('E2').setValue(avg_con);
    mainTankLog.getRange('F2').setValue(0);
    mainTankLog.getRange('G2').setValue(0);
    mainTankLog.getRange('H2').setValue(0);
  }
  SpreadsheetApp.flush();
  // return "Updated Main Tank level Successfully";
  return blynkData(val_3, val_4);
}

function div_1_tank(values) {
  var dataArr = values.split(",");
  var val_1 = dataArr [0]; // val_1 from Arduino code - tank level
  var val_2 = dataArr [1]; // val_2 from Arduino code - tank Liters

  var date_A2 = div1TankLog.getRange('A2').getValue();
  var past_Date = Utilities.formatDate(date_A2, "GMT+5:30", "yyyy/MM/dd");
  if ( past_Date.toString().match(Curr_Date.toString()) ) {
    var preLit = parseInt( div1TankLog.getRange('D2').getValue() );
    var ci = calcConsInta(preLit, val_2).split(",");
    var con = parseInt(ci[0]) + parseInt( div1TankLog.getRange('F2').getValue() );
    var inta = parseInt(ci[1]) + parseInt( div1TankLog.getRange('G2').getValue() );
    div1TankLog.getRange('B2').setValue(Curr_Time);
    div1TankLog.getRange('C2').setValue(val_1);  
    div1TankLog.getRange('D2').setValue(val_2);
    div1TankLog.getRange('F2').setValue(con);
    div1TankLog.getRange('G2').setValue(inta);
  }
  else {
    var sum = 0;
    for (var i = 3 ; i < 3+7 ; i++){
      var val = div1TankLog.getRange(i, 6).getValues();
      sum = sum + parseInt(val);
    }
    var avg_con = sum / 7;
    div1TankLog.insertRows(2); // insert full row directly below header text
    div1TankLog.getRange('A2').setValue(Curr_Date);
    div1TankLog.getRange('B2').setValue(Curr_Time);
    div1TankLog.getRange('C2').setValue(val_1);
    div1TankLog.getRange('D2').setValue(val_2);
    div1TankLog.getRange('E2').setValue(avg_con);
    div1TankLog.getRange('F2').setValue(0);
    div1TankLog.getRange('G2').setValue(0);
    div1TankLog.getRange('H2').setValue(0);
  }
  SpreadsheetApp.flush();
  return "Updated Div 1 Tank level Successfully";
}

function calcConsInta(preLit, currLit) {
  var con = 0;
  var inta = 0;
  var dif = preLit - currLit;
  if (dif < 0) {
    inta = dif * (-1);
  } else {
    con = dif;
  }
  return con.toString() + "," + inta.toString();
}

function blynkData(start, stop) {
  var m_avg_con = mainTankLog.getRange('E2').getValue();
  var m_tot_con = mainTankLog.getRange('F2').getValue();
  var m_d1_con  = mainTankLog.getRange('G2').getValue();

  var d1_lev = div1TankLog.getRange('C2').getValue();
  var d1_lit = div1TankLog.getRange('D2').getValue();
  var d1_rec = div1TankLog.getRange('G2').getValue();

  var time_arr = Curr_Time.split(":");
  var t_arr = time_arr[2].split(" ");
  var prs_hh = parseInt(time_arr[0]);
  var prs_mm = parseInt(time_arr[1]);
  var prs_ss = parseInt(t_arr[0]);
  if (t_arr[1].toString().match("PM")){
    prs_hh = prs_hh + 12;
  }
  var blynk_time = `${prs_hh}:${prs_mm}:${prs_ss}`;

  prs_mm = (prs_hh * 60) + prs_mm;

  var st_time_arr = start.split(":");
  var st_mm = (parseInt(st_time_arr[0]) * 60) + parseInt(st_time_arr[1]);

  var ed_time_arr = stop.split(":");
  var ed_mm = (parseInt(ed_time_arr[0]) * 60) + parseInt(ed_time_arr[1]);

  var valveFlag = parseInt( div1TankLog.getRange('H2').getValue() );
  var auto = 0;
  if (d1_lev < 20 || valveFlag==1 ) {
    auto = 1;
    valveFlag = 1;
  }
  if (d1_lev > 80 && valveFlag==1) {
    auto = 0;
    valveFlag = 0;
  }
  if (st_mm < prs_mm && prs_mm < ed_mm) {
    auto = 1;
  }
  div1TankLog.getRange('H2').setValue(valveFlag);

  var m_tk_lev  = mainTankLog.getRange('C2').getValue();
  if (m_tk_lev < 20) {
    auto = 0;
  }
  
  var str = `{ "date":"${Curr_Date}", "time":"${blynk_time}", "m_avg_con":${m_avg_con}, "m_tot_con":${m_tot_con}, "m_d1_con":${m_d1_con}, "d1_lev":${d1_lev}, "d1_lit":${d1_lit}, "d1_rec":${d1_rec}, "auto":${auto} }`;
  
  SpreadsheetApp.flush();
  return str;
}

function sendToMongoose() {
  var m_tk_lev  = mainTankLog.getRange('C2').getValue();
  var m_tk_lit  = mainTankLog.getRange('D2').getValue();
  var m_avg_con = mainTankLog.getRange('E2').getValue();
  var m_tot_con = mainTankLog.getRange('F2').getValue();
  var m_d1_con  = mainTankLog.getRange('G2').getValue();
  var m_tk_rec  = mainTankLog.getRange('H2').getValue();

  var d1_tk_lev  = div1TankLog.getRange('C2').getValue();
  var d1_tk_lit  = div1TankLog.getRange('D2').getValue();
  var d1_avg_con = div1TankLog.getRange('E2').getValue();
  var d1_tot_con = div1TankLog.getRange('F2').getValue();
  var d1_tk_rec  = div1TankLog.getRange('G2').getValue();
  
  var url_base = `https://subanesh.cyclic.app/gls_callback?date=${Curr_Date}&time=${Curr_Time}`;
  var query_str = `m_tk_lev=${m_tk_lev}&m_tk_lit=${m_tk_lit}&m_avg_con=${m_avg_con}&m_tot_con=${m_tot_con}&m_d1_con=${m_d1_con}&m_tk_rec=${m_tk_rec}&d1_tk_lev=${d1_tk_lev}&d1_tk_lit=${d1_tk_lit}&d1_avg_con=${d1_avg_con}&d1_tot_con=${d1_tot_con}&d1_tk_rec=${d1_tk_rec}`;
  UrlFetchApp.fetch(url_base+"&"+query_str);
  SpreadsheetApp.flush();
}