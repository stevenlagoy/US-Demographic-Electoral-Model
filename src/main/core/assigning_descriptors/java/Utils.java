import java.util.Collection;
import java.util.Random;

public class Utils {

    /**
     * Selects a float between 0.0 and 1.0 which can be used as a percentage.
     * 
     * @return A pseudorandomly selected float in the range [0.0, 1.0)
     * @see #randPercent(float, float)
     */
    public static float randPercent() {
        return randFloat(0.0f, 1.0f);
    }

    /**
     * Selects a float between the min and the max.
     * <p>
     * <i>If min is a larger value than max, their values will be swapped.</i>
     * 
     * @param min The minimum value which can be selected (inclusive)
     * @param max The maximum value which can be selected (exclusive)
     * @return A pseudorandomly selected float in the range [min, max)
     */
    public static float randFloat(float min, float max) {
        // will perform the same if min and max are flipped
        Random rand = new Random();
        // return a float between min and max (exclusive), equally distributed
        return (max - min) * rand.nextFloat() + min;
    }

    public static boolean randChance(float chance) {
        if (chance <= 0.0f)
            return false;
        else if (chance >= 1.0f)
            return true;
        else
            return randPercent() <= chance;
    }

    /**
     * Selects a double between the min and the max.
     * 
     * @param min The minimum value which can be selected (inclusive)
     * @param max The maximum value which can be selected (exclusive)
     * @return A pseudorandomly selected double in the range [min, max)
     */
    public static double randDouble(double min, double max) {
        // will perform the same if min and max are flipped
        Random rand = new Random();
        return (max - min) * rand.nextDouble() + min; // return a double between min and max (exclusive), equally
                                                      // distributed
    }

    /**
     * Selects one value from an array.
     * 
     * @param items The array to select from.
     * @return One randomly selected value.
     */
    public static <T> T randSelect(T[] items) {
        if (items.length == 0)
            return null;
        Random rand = new Random();
        int randomNumber = rand.nextInt(items.length);
        return items[randomNumber];
    }

    /**
     * Selects one value from a collection.
     * 
     * @param items The collection to select from.
     * @return One randomly selected value.
     */
    public static <T> T randSelect(Collection<T> items) {
        if (items == null || items.size() == 0)
            return null;

        Random rand = new Random();
        int randomNumber = rand.nextInt(items.size());
        int i = 0;
        for (T item : items) {
            if (i == randomNumber)
                return item;
            i++;
        }
        return null; // Never reached when items.size() > 0
    }

}
