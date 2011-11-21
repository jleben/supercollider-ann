AnnDef {
  var <num, <filename;

  *new { arg num, filename;
    ^super.new.initAnnDef( num, filename );
  }

  initAnnDef { arg num_, filename_;
    num = num_;
    filename = filename_;
  }

  load { arg server, file_name = filename, out_count;
    filename = file_name;
    server.sendMsg( \cmd, \loadAnn, num, filename );
  }
}


Ann : MultiOutUGen {
  classvar <outc;

/*
  *ar { arg annFileNum, outCount ... inputs;
    outCount = max(1,outCount);
    outc = outCount;
    ^this.multiNewList(['audio', annFileNum, outCount] ++ inputs);
  }

  *kr { arg annFileNum, outCount ... inputs;
    outCount = max(1,outCount);
    outc = outCount;
    ^this.multiNewList(['control', annFileNum, outCount] ++ inputs);
  }
*/

  *kr { arg annDefNum, outCount, input, period = 20;
    //"kr".postln;
    outCount = max(1,outCount);
    outc = outCount;
    ^this.multiNew('control', annDefNum, input, period);
  }

  init { arg ... ins;
    //"init".postln;
    inputs = ins;
    ^this.initOutputs(Ann.outc, rate);
  }
}
