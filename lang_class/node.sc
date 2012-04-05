AnnNode {
    classvar <nodes;

    var <id;
    var <server;
    var <obus;
    var <synthdef;
    var <synth;

    *initClass {
        Class.initClassTree(IdentityDictionary);
        nodes = IdentityDictionary();
    }

    *new { arg id, synthdef, numChannels, server;
        var existing = nodes[id];
        if(existing.notNil) {existing.free};
        ^super.new.initAnnNode(id, synthdef, numChannels, server);
    }

    initAnnNode { arg id_, sdef_, n_chan_, srv_;
        AnnNode.nodes.put(id_, this);

        id = id_;
        synthdef = sdef_;
        server = srv_ ?? {Server.default()};

        obus = Bus.control(server, n_chan_);
    }

    play { arg params;
        if(synth.notNil) {
            "WARNING: AnnNode already playing. Stop first.".warn
        }{
            if(synthdef.class === SynthDef){
                synth = synthdef.play(server, [\ibus, obus] ++ params, addAction:\addToTail)
            }{
                synth = Synth.tail(server, synthdef, [\ibus, obus] ++ params);
            }
        };
    }

    stop {
        if(synth.notNil) {synth.free; synth = nil};
    }

    free {
        if(obus.notNil) {obus.free; obus = nil};
        if(synth.notNil) {synth.free; synth = nil};
    }
}

AnnChildNode : AnnNode {

    var <parent;
    var <ibus;
    var <ann, <ann_id, <td;
    var <ann_synth, <ann_synthdef;
    var <control = false;

    *new { arg id, ann, parent, synthdef, server;
        if( ann.numInputs != parent.obus.numChannels ) {
            Error("ANN input count does not match parent's channel count.").throw;
        };
        ^super.new(id, synthdef, ann.numOutputs, server).initAnnChildNode( ann, parent );
    }

    initAnnChildNode { arg ann_, parent_;
        parent = parent_;
        ibus = parent.obus;
        ann = ann_;
        ann_id = Fann.allocId;
        ann_synthdef = SynthDef('ann_' ++ ann_id, {
            var in = In.kr( ibus, ibus.numChannels);
            var ann = AnnBasic.kr(ann_id, in, obus.numChannels);
            Out.kr(obus, ann);
        }).send(server);
    }

    clearTd { td = nil; ann.reset; }

    train { arg mse=0.01, period=100, maxEpochs=5000, action;
        var in, out;
        var send_routine = Routine({
            "sending ann".postln;
            ann.send(ann_id, server);
            server.sync;
            action.value;
        });
        in = ibus.getn( ibus.numChannels, { |val|
            "got ibus".postln;
            in = val;
            obus.getn( obus.numChannels, { |val|
                "got obus".postln;
                out = val;
                td = td.add(in ++ out);
                ann.reset;
                ann.trainingData = td;
                ann.train(mse, period, maxEpochs, {send_routine.play(AppClock)} );
            });
        });
    }

    play { arg params;
        var def_name;
        if(synth.notNil) {"WARNING: AnnNode already playing. Stop first.".warn};

        ann_synth = Synth.newPaused(ann_synthdef.name, nil, server);

        def_name = if(synthdef.class === SynthDef){synthdef.name}{synthdef};
        synth = Synth.after(ann_synth, def_name, [\ibus, obus] ++ params);
    }

    playAnn { arg on;
        if(ann_synth.notNil) {
            ann_synth.run(on);
        }
    }

    stop {
        if(ann_synth.notNil) {ann_synth.free; ann_synth = nil};
        super.stop;
    }

    free {
        if(ann_synth.notNil) {ann_synth.free; ann_synth = nil};
        if(ann_id.notNil) {Fann.freeId(ann_id); ann_id = nil};
        super.free;
    }
}
