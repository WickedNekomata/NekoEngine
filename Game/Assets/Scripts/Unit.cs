using System.Collections;
using JellyBitEngine;

public class Unit : JellyScript
{
    // General Stats
    public int damage = 25;
    public float mov_speed = 1.0f;

    // Life
    private int current_life = 100;
    public int max_life = 100;

    // Start is called before the first frame update
    public override void Start()
    {

    }

    public void Hit(int damage)
    {
        current_life -= damage;

        if (current_life <= 0)
            Destroy(gameObject);
    }

}
