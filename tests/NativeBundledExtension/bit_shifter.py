import extsrc/BitShifter

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("A ruby service testing the embedding of native extensions")
@browserplus.bp_doc("left", "left shift an integer one bit\n\
                             <num: integer> what to rotate left")
@browserplus.bp_doc("right", "right shift an integer one bit\n\
                              <num: integer> what to rotate right")
class Shifty:
    def left(self, trans, args):
        bs = BitShifter.new
        trans.complete(bs.left args[:num])


    def right(self, trans, args):
        bs = BitShifter.new
        trans.complete(bs.right args[:num])
