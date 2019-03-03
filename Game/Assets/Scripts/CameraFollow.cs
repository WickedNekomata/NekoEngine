using System.Collections;
using JellyBitEngine;

public class CameraFollow : JellyScript
{
    private Vector3 starting_pos = null;

	public GameObject alita;


    public override void Update()
    {
        if (starting_pos == null)
            starting_pos = gameObject.transform.position;

        gameObject.transform.position = starting_pos + alita.transform.position;
    }
}

