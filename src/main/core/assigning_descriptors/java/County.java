import java.util.ArrayList;
import java.util.List;

public class County {

    private List<Descriptor> descriptors;
    private String name;
    private String countyFIPS;
    private long population;
    private double score;

    public County(String name, String countyFIPS, long population) {
        this.name = name;
        this.countyFIPS = countyFIPS;
        this.population = population;
        this.descriptors = new ArrayList<>();
    }

    public String getName() {
        return name;
    }

    public List<Descriptor> getDescriptors() {
        return descriptors;
    }

    public void setDescriptors(List<Descriptor> descriptors) {
        this.descriptors = new ArrayList<>(descriptors);
    }

    public boolean addDescriptor(Descriptor d) {
        return descriptors.add(d);
    }

    public boolean removeDescriptor(Descriptor d) {
        return descriptors.remove(d);
    }

    public boolean hasDescriptor(Descriptor d) {
        return descriptors.contains(d);
    }

    public boolean addOrRemoveDescriptor(Descriptor d) {
        if (hasDescriptor(d)) {
            removeDescriptor(d);
            return false;
        } else {
            addDescriptor(d);
            return true;
        }
    }

    public String getCountyFips() {
        return countyFIPS;
    }

    public long getPopulation() {
        return population;
    }

    public double getScore() {
        return score;
    }

    public void recalculate() {

    }

    @Override
    public String toString() {
        return String.format("%s (%s)", name, countyFIPS);
    }
}