import java.util.ArrayList;
import java.util.List;

public class Simulation {

    // STATIC FINALS ---------------------------------------------------------------

    public static final long MAX_ITER = 10_000_000L;
    public static final int MAX_TRIES = 500_000;

    public static final float DESCRIPTOR_PERMUTE_CHANCE = 0.99f;

    public static final double MAX_CHANGE_AMT = 0.25;

    // INSTANCE VARIABLES ----------------------------------------------------------

    private long nationalPopulation;
    private List<County> counties;
    private List<Descriptor> descriptors;
    private List<String> demographicNames;

    public Simulation() {
        nationalPopulation = 0L;
        counties = new ArrayList<>();
        descriptors = new ArrayList<>();
        demographicNames = new ArrayList<>();
    }

    public void initialize() {
        counties.clear();

    }

    class Change {
        public Runnable undoFn;

        public Change(Runnable fn) {
            this.undoFn = fn;
        }

        void undo() {
            if (undoFn != null)
                undoFn.run();
        }
    }

    public void run() {
        int tries = 0;
        Change ch;
        double prevScore = 0.0, newScore = 0.0;
        for (long iter = 0L; iter < MAX_ITER && tries < MAX_TRIES; iter++) {
            if (Utils.randChance(DESCRIPTOR_PERMUTE_CHANCE)) {
                // Choose a descriptor to modify
                Descriptor d = Utils.randSelect(descriptors);
                // Choose an effect to modify
                String e = Utils.randSelect(d.getEffects().keySet());
                // Choose an amount by which to modify
                double c = Utils.randDouble(-MAX_CHANGE_AMT, MAX_CHANGE_AMT);
                final double prev = d.getEffect(e);
                // Make the change
                d.addEffect(e, c);
                ch = new Change(() -> {
                    d.setEffect(e, prev);
                });
            } else {
                // Choose a county to modify
                County c = Utils.randSelect(counties);
                // Choose a membership-modifiable descriptor
                Descriptor d;
                do {
                    d = Utils.randSelect(descriptors);
                } while (d == null || !d.isMembershipModifiable());
                // If county is a member, revoke membership
                // If county is not a member, grant membership
                final Descriptor dF = d;
                c.addOrRemoveDescriptor(dF);
                ch = new Change(() -> {
                    c.addOrRemoveDescriptor(dF);
                });
            }

            // Evaluate
            newScore = score();
            if (newScore < prevScore) { // Change did not make model better
                // Revert the change
                ch.undo();
                ++tries;
            } else {
                prevScore = newScore;
                tries = 0;
            }

            // Print details
            if (iter % 10_000 == 0) {
                System.out.printf("Iter: %8d, Acc: %12d%% %n", iter, newScore * 100);
            }
        }
    }

    public double score() {
        double score = 0.0;
        for (County c : counties) {
            score += (c.getScore() * (c.getPopulation() * 1.0 / nationalPopulation));
        }
        return score;
    }

    // GETTERS AND SETTERS ---------------------------------------------------------

    public long getNationalPopulation() {
        return nationalPopulation;
    }

    public List<County> getCounties() {
        return counties;
    }

    public boolean hasCounty(County c) {
        return counties.contains(c);
    }

    public boolean addCounty(County c) {
        return counties.add(c);
    }

    public boolean removeCounty(County c) {
        return counties.remove(c);
    }

    public List<Descriptor> getDescriptors() {
        return descriptors;
    }

    public boolean hasDescriptor(Descriptor d) {
        return descriptors.contains(d);
    }

    public boolean addDescriptor(Descriptor d) {
        return descriptors.add(d);
    }

    public boolean removeDescriptor(Descriptor d) {
        return descriptors.remove(d);
    }

    public List<String> getDemographicNames() {
        return demographicNames;
    }
}
