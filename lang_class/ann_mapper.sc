AnnMapper {
    var n_in, n_out;
    var <ann;
    var <td;

    *new { arg ...layers;
        ^super.new.initAnnMapper(layers);
    }

    *load { arg filename;
        ^super.new.initLoadAnn(filename);
    }

    initAnnMapper { arg layers;
        ann = Fann(*layers);
        n_in = ann.numInputs;
        n_out = ann.numOutputs;
    }

    initLoadAnn { arg file;
        ann = Fann.load(file);
        n_in = ann.numInputs;
        n_out = ann.numOutputs;
    }

    add { arg input, output;
        if ((input.size != n_in) || (output.size != n_out)) {
            Error("Input or output count mismatch").throw;
        };

        td = td.add(input ++ output);
    }

    removeAt { arg index; td.removeAt(index); }

    removeAll { td = nil; }

    train { arg targetError = 0.01, period = 10, maxEpochs, action;
        ann.reset;
        ann.trainingData = td;
        ann.train(targetError, period, maxEpochs, action);
    }

    run { arg input, output;
        ^ann.run(input, output);
    }
}
