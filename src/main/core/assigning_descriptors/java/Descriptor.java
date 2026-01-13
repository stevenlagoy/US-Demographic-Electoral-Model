import java.util.Map;

public class Descriptor {

    private String name;
    private Map<String, Double> effects;
    private final boolean membershipModifiable;

    public Descriptor(String name) {
        this(name, true);
    }

    public Descriptor(String name, boolean membershipModifiable) {
        this.name = name;
        this.membershipModifiable = membershipModifiable;
    }

    public String getName() {
        return name;
    }

    public Map<String, Double> getEffects() {
        return effects;
    }

    public double getEffect(String demographic) {
        return effects.get(demographic);
    }

    public double setEffect(String demographic, double effect) {
        return effects.put(demographic, effect);
    }

    public double addEffect(String demographic, double effect) {
        double prev = effects.get(demographic);
        return effects.put(demographic, effect + prev);
    }

    public boolean isMembershipModifiable() {
        return membershipModifiable;
    }

}
