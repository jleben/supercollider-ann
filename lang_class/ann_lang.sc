Fann {
	classvar serverIdAllocator;

	var finalizer, data;
	var trainer;

	*initClass {
		Class.initClassTree(StackNumberAllocator);
		serverIdAllocator = StackNumberAllocator(0,199);
	}

	*allocId { ^serverIdAllocator.alloc; }
	*freeId { arg id; ^serverIdAllocator.free(id); }

	*activationFunctionNames {
		_Ann_GetActivationFuncNames
	}

	*new { arg ... structure;
		^super.new.initAnn( structure );
	}

	*load { arg filename;
		^super.new.prLoad(filename);
	}

    send { arg num, server = Server.default();
        var filename = Fann.prMakeFilename(num);
        this.save(filename);
        server.sendMsg( "/cmd", \loadAnn, num, filename );
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

	train { arg finalMSE = 0.01, period = 10, maxEpochs;
		var mse;
		var count;
		var iter;

		period = max(period,1);

        trainer = {
            mse = this.trainOneEpoch;
            iter = 1;
            count = period - 1;
            while { (mse > finalMSE) and: (maxEpochs.isNil or: {iter < maxEpochs}) } {
                if( count <= 0 ) {
                    (iter.asString ++ ": " ++ mse.asString).postln;
                    count = period;
                    0.yield;
                };
                mse = this.trainOneEpoch;
                count = count - 1;
                iter = iter + 1;
            };
            (iter.asString ++ ": " ++ mse.asString).postln;
            if( mse <= finalMSE ) {"done".postln} {"failed".postln};
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

    reset { _Ann_Reset }

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
