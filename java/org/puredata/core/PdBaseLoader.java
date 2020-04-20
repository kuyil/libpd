/**
 * 
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see the
 * file, "LICENSE.txt," in this distribution.
 * 
 */

package org.puredata.core;

/**
 * PdBase natives loader.
 * 
 * @author mgsx
 *
 */
public abstract class PdBaseLoader {

    /**
     * Load required native libraries prior to use {@link PdBase}.
     */
    public abstract void load();

    /**
     * Default {@link PdBaseLoader}. Called when {@link PdBase} is first accessed.
     * This allows for implementing a custom loader depending on libpd use context. To do so,
     * first change this {@link PdBaseLoader} with your own one prior to call any {@link PdBase} methods.
     */
    public static PdBaseLoader loaderHandler = new PdBaseLoader() {
        @Override
        public void load() {
            try {
                Class<?> inner[] = Class.forName("android.os.Build").getDeclaredClasses();
                // Now we know we're running on an Android device.
                System.loadLibrary("pd");
                int version = -1;
                for (Class<?> c : inner) {
                    if (c.getCanonicalName().equals("android.os.Build.VERSION")) {
                        try {
                            version = c.getDeclaredField("SDK_INT").getInt(null);
                        } catch (Exception e) {
                            version = 3; // SDK_INT is not available for Cupcake.
                        }
                        break;
                    }
                }

                final int oboeMinAndroidVersion = 16;  // ref: https://github.com/google/oboe/blob/master/README.md
                final int openslMinAndroidVersion = 9; // ref: https://developer.android.com/ndk/guides/audio/opensl/opensl-for-android

                if (version >= oboeMinAndroidVersion) {
                    // As per android documentation:
                    // Old versions of Android had bugs in PackageManager and the dynamic
                    // linker that caused installation, update, and loading of native libraries
                    // to be unreliable. In particular, if your app targets a version of
                    // Android earlier than Android 4.3 (Android API level 18), and you use
                    // libc++_shared.so, you must load the shared library before
                    // any other library that depends on it.
                    // ref: https://developer.android.com/ndk/guides/cpp-support
                    final int libcppSharedAutomaticLoadAndroidVersion = 18; 
                    if (version < libcppSharedAutomaticLoadAndroidVersion) {
                        System.loadLibrary("c++_shared");
                    }
                   
                    System.out.println("loading pdnativeoboe for Android");
                    System.loadLibrary("pdnativeoboe");
                    // System.out.println("loading pdnativeopensl for Android");
                    // System.loadLibrary("pdnativeopensl");
                } else if (version >= openslMinAndroidVersion) {
                    System.out.println("loading pdnativeopensl for Android");
                    System.loadLibrary("pdnativeopensl");
                } else {
                    System.out.println("loading pdnative for Android");
                    System.loadLibrary("pdnative");
                }
            } catch (Exception e) {
                // Now we know we aren't running on an Android device.
                NativeLoader.loadLibrary("libwinpthread-1", "windows");
                NativeLoader.loadLibrary("pdnative");
            }
        }
    };
}
