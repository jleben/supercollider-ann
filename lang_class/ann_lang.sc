Fann {
	var finalizer, data;
	var trainer;

	*activationFunctionNames {
		_Ann_GetActivationFuncNames
	}

	*new { arg ... structure;
		^super.new.initAnn( structure );
	}

	*load { arg filename;
		^super.new.prLoad(filename);
	}

	send { arg server, num;
		var filename = this.prMakeFilename(num);
		this.save(filename);
		server.sendMsg( \cmd, \loadAnn, num, filename );
	}

	numInputs { _Ann_InputCount }

	numOutputs { _Ann_OutputCount }

	activationFunction_ { arg name, layers=\all; //a symbol, one of *activationFunctionNames
		_Ann_SetActivationFunc
	}

	trainingData_ { arg trainData;
		this.prSetTrainData( trainData );
	}

	trainOneEpoch {
		_Ann_Train
		^this.primitiveFailed;
	}

	train { arg finalMSE = 0.01, period = 10;
		var mse;
		var count;
		var iter = 0;

		period = max(period,1);

		trainer = {
			mse = this.trainOneEpoch;
			while { mse > finalMSE} {
				(iter.asString ++ ": " ++ mse.asString).postln;
				count = period;
				while {
					(mse > finalMSE) && (count > 0);
				}{
					mse = this.trainOneEpoch;
					count = count - 1;
					iter = iter + 1;
				};
				if( mse > finalMSE ) {0.yield};
			};
			(iter.asString ++ ": " ++ mse.asString).postln;
			"done".postln;
			trainer = nil;
		}.fork(AppClock);
	}

	stop {
		if( trainer.notNil ) {
			trainer.stop;
			trainer = nil;
		}
	}

	run { arg input, output;
		_Ann_Run
		^this.primitiveFailed;
	}

	save { arg filename;
		_Ann_Save
		^this.primitiveFailed;
	}

// PRIVATE

	initAnn { arg structure;
		this.prCreateAnn( structure );
	}

	prCreateAnn { arg structure;
		_Ann_Create
		^this.primitiveFailed;
	}

	prSetTrainData { arg td;
		_Ann_SetTrainingData
		^this.primitiveFailed;
	}

	prLoad { arg filename;
		_Ann_Load
		^this.primitiveFailed
	}

	*prMakeFilename { arg num;
		^"/home/jakob/.local/share/SuperCollider/ann_defs" +/+ "ann_def_" ++ num.asString
	}
}
