using System.Collections;
using JellyBitEngine;

public class Blood : JellyScript
{
    ParticleEmitter part;

   public void PartEmit()
    {
        part = gameObject.GetComponent<ParticleEmitter>();
        part.Play();
    }
}


