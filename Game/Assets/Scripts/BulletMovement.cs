using System.Collections;
using JellyBitEngine;


class BulletMovement : JellyScript
{
    public static GameObject direction;
    public float speed = 25f;
    public float life = 10f;

    private float lifeTime = 0f;
    private Vector3 dir = new Vector3(0, 0, 0);
    private bool getDirection = true;


    public override void Awake()
    {
        lifeTime = Time.time;
    }

    public override void Update()
    {
        if (getDirection)
        {
            dir = (direction.transform.position - transform.position).normalized();

            getDirection = false;
        }

        transform.position += dir * (speed * Time.deltaTime);

        if (Time.time > lifeTime + life)
        {
            Destroy(gameObject);
        }
    }
}