<!-- # Poly External Scale format

In my modules, I frequently make use of the ability to quantize to a scale or chord, using a polyphonic cable to transfer external scales between devices. I am calling it the "Poly External Scale" format. I encourage other developers to use it, if it makes sense for their module.

- The format operates on a single twelve tone octave. It carries no information _which_ octave it is.
- There's 12 channels, one per semitone of the octave. The first channel is C, the second C#, and so on.
- When a channel is unplugged, or recives 0V or fewer, the corresponding semitone is disabled.
- When a channel receives anything above 0V, the corresponding semitone is enabled. There's no in-between states.
- Modules capable of communicating an external scale output 0V for disabled semitones and 10V for enabled semitones (following the usual robustness adage "liberal in what you accept, conservative in what you send").
- Name jacks that support the format something ranging in complexity from "Poly External Scale" to "Ext."
- In the documentation, name it "Poly External Scale", and mention it's compatible with modules by other developers.

This format is so self-evident, I can't claim any degree of ownership over it. I just think it would be useful for users to have a consistent naming scheme for it, so I'm proposing one. When users see "Poly External Scale", they know it's compatible.

A scale is just 12 bits of data: it would have been easy to encode it as voltage on a monophonic channel. It would be a higher performance solution than the Poly External Scale format. However, such a format would not be intuitively hackable by the user. That matters a lot more to me. Users can understand Poly External Scales simply by observing and altering their values, they don't have to read this document or do maths to understand them. -->
