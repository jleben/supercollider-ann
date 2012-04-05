
AnnBasic : MultiOutUGen {
  *kr { arg annDefNum, inputs, outCount;
    ^this.multiNewList(['control', annDefNum] ++ inputs ++ outCount);
  }

  init { arg ... ins;
    inputs = ins;
    ^this.initOutputs(ins[ins.size-1], rate);
  }
}

AnnTime : UGen {
  *ar { arg annDefNum, winSize, in;
    ^this.multiNew('audio', annDefNum, winSize, in);
  }

  *kr { arg annDefNum, winSize, in;
    ^this.multiNew('control', annDefNum, winSize, in);
  }
}

AnnAutoTrainer : UGen {
  *kr { arg annDefNum, inputs, train=0, numSets=20, desiredMSE=0.01;
    ^this.multiNewList(['control', annDefNum] ++ inputs ++ [train, numSets, desiredMSE]);
  }
}
