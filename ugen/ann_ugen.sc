
Ann : MultiOutUGen {
  classvar <outc;

  // 'period' = number of computation cycles between each sampling
  //  of the input and running of ANN
  *kr { arg annDefNum, outCount, input, period = 20;
    outCount = max(1,outCount);
    outc = outCount;
    ^this.multiNew('control', annDefNum, input, period);
  }

  init { arg ... ins;
    inputs = ins;
    ^this.initOutputs(Ann.outc, rate);
  }
}

AnnBasic : MultiOutUGen {
  *kr { arg annDefNum, inputs, outCount;
    ^this.multiNewList(['control', annDefNum] ++ inputs ++ outCount);
  }

  init { arg ... ins;
    inputs = ins;
    ^this.initOutputs(ins[ins.size-1], rate);
  }
}

AnnAutoTrainer : UGen {
  *kr { arg annDefNum, inputs, train=0, numSets=20, desiredMSE=0.01;
    ^this.multiNewList(['control', annDefNum] ++ inputs ++ [train, numSets, desiredMSE]);
  }
}
