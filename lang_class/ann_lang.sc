Fann {
	var finalizer, data;
	var struct;
	var trainer;

	*new { arg ... structure;
		^super.new.initAnn( structure );
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

	layers { ^struct.copy }

// PRIVATE

	initAnn { arg structure;
		this.prCreateAnn( structure );
		struct = structure;
	}

	prCreateAnn { arg structure;
		_Ann_Create
		^this.primitiveFailed;
	}

	prSetTrainData { arg td;
		_Ann_SetTrainingData
		^this.primitiveFailed;
	}


}
