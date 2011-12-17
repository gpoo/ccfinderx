package utility;

// borrowed from http://www.cuvee.org/index.php?Generics%A4%C8Clone

public interface ExCloneable<T extends ExCloneable<T>> extends Cloneable {
    public T clone();
}