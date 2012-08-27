/* 
 * File:   main.cpp
 * Author: themylogin
 *
 * Created on 27 Август 2012 г., 2:34
 */


#include <boost/thread.hpp>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#include "../shared/theLEDStripControlDescription.hpp"
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include <alsa/asoundlib.h>

#include <string.h>
#include <unistd.h>

/*
 * 
 */
class RGBPlugin
{
public:
    virtual ~RGBPlugin(){}    
    virtual void processRGB(uint8_t& r, uint8_t& g, uint8_t& b) = 0;
};

/*
 * 
 */
class NoneRGBPlugin : public RGBPlugin
{
public:
    virtual void processRGB(uint8_t& r, uint8_t& g, uint8_t& b){}
};

/*
 * 
 */
namespace AudioRGBPlugins
{    
    /*
     * 
     */
    class Exception : public std::exception
    {
    public:
        Exception(std::string function, std::string error) throw()
        {
            sprintf(this->message, "error in %s: %s", function.c_str(),
                    error.c_str());
        }

        virtual const char* what() const throw()
        {
            return this->message;
        }

    private:
        char message[1024];
    };

    /*
     * 
     */
    class Plugin : public RGBPlugin
    {
    public:
        Plugin()
        {
            int err;
            snd_pcm_hw_params_t* hw_params;

            if ((err = snd_pcm_open(&this->capture_handle, "hw:0",
                    SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK)) < 0)
            {
                throw Exception("snd_pcm_open", snd_strerror(err));
            }

            if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
            {
                snd_pcm_close(this->capture_handle);
                throw Exception("snd_pcm_hw_params_malloc", snd_strerror(err));
            }

            if ((err = snd_pcm_hw_params_any(this->capture_handle,
                    hw_params)) < 0)
            {
                snd_pcm_hw_params_free(hw_params);
                snd_pcm_close(this->capture_handle);
                throw Exception("snd_pcm_hw_params_any", snd_strerror(err));
            }

            if ((err = snd_pcm_hw_params_set_access(this->capture_handle,
                    hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
            {
                snd_pcm_hw_params_free(hw_params);
                snd_pcm_close(this->capture_handle);
                throw Exception("snd_pcm_hw_params_set_access",
                        snd_strerror(err));
            }

            if ((err = snd_pcm_hw_params_set_format(this->capture_handle,
                    hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
            {
                snd_pcm_hw_params_free(hw_params);
                snd_pcm_close(this->capture_handle);
                throw Exception("snd_pcm_hw_params_set_format",
                        snd_strerror(err));
            }

            unsigned int rate = 44100;
            if ((err = snd_pcm_hw_params_set_rate_near(this->capture_handle,
                    hw_params, &rate, 0)) < 0)
            {
                snd_pcm_hw_params_free(hw_params);
                snd_pcm_close(this->capture_handle);
                throw Exception("snd_pcm_hw_params_set_rate_near",
                        snd_strerror(err));
            }

            if ((err = snd_pcm_hw_params_set_channels(this->capture_handle,
                    hw_params, 2)) < 0)
            {
                snd_pcm_hw_params_free(hw_params);
                snd_pcm_close(this->capture_handle);
                throw Exception("snd_pcm_hw_params_set_channels",
                        snd_strerror(err));
            }

            if ((err = snd_pcm_hw_params(this->capture_handle, hw_params)) < 0)
            {
                snd_pcm_hw_params_free(hw_params);
                snd_pcm_close(this->capture_handle);
                throw Exception("snd_pcm_hw_params", snd_strerror(err));
            }

            snd_pcm_hw_params_free(hw_params);

            if ((err = snd_pcm_prepare(this->capture_handle)) < 0)
            {
                snd_pcm_close(this->capture_handle);
                throw Exception("snd_pcm_prepare", snd_strerror(err));
            }
        }
        
        ~Plugin()
        {
            snd_pcm_close(this->capture_handle);
        }
    
    protected:
        unsigned int dataSize;
        int16_t leftData[44100];
        int16_t rightData[44100];
        
        void capture()
        {
            int err;
            int16_t buffer[88200];
            while ((err = snd_pcm_readi(this->capture_handle, buffer, 88200))
                    == -EAGAIN);
            if (err < 0)
            {
                throw Exception("snd_pcm_readi", snd_strerror(err));
            }
            
            this->dataSize = err / 2;
            for (unsigned int i = 0; i < this->dataSize; i++)
            {
                this->leftData[i] = buffer[i * 2];
                this->rightData[i] = buffer[i * 2 + 1];
            }
        }
        
        void rectify()
        {
            for (unsigned int i = 0; i < this->dataSize; i++)
            {
                this->leftData[i] = abs(this->leftData[i]);
                this->rightData[i] = abs(this->rightData[i]);
            }
        }
        
        void lowPassFilter()
        {
            for (unsigned int i = 2; i < this->dataSize / 2; i++)
            {
                this->leftData[i] = (0.33 * this->leftData[i] +
                        this->leftData[i - 1] +
                                0.5 * this->leftData[i - 2]) / 1.83;
                this->rightData[i] = (0.33 * this->rightData[i] +
                        this->rightData[i - 1] +
                                0.5 * this->rightData[i - 2]) / 1.83;
            }
        }
        
        float amplitude()
        {
            double sum = 0;
            for (unsigned int i = 0; i < this->dataSize; i++)
            {
                sum += 0.5 * this->leftData[i] * this->leftData[i];
                sum += 0.5 * this->rightData[i] * this->rightData[i];
            }
            
            static double sums[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
            static int sizes[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
            for (int i = 0; i < 10; i++)
            {
                sums[i] = sums[i + 1];
                sizes[i] = sizes[i + 1];
            }
            sums[9] = sum;
            sizes[9] = this->dataSize;
            
            sum = 0;
            int size = 0;
            for (int i = 0; i < 10; i++)
            {
                sum += sums[i];
                size += sizes[i];
            }
            
            float vu = 6 * log(sqrt(sum / size)) / log(2);
            
            static float min_vu = std::numeric_limits<float>::infinity();
            static float max_vu = 0.0;
            if (vu < min_vu)
            {
                min_vu = vu;
            }
            if (vu > max_vu)
            {
                max_vu = vu;
            }
            return (vu - min_vu) / (max_vu - min_vu);
        }

    private:
        snd_pcm_t* capture_handle;
    };
    
    class VU : public Plugin
    {
    public:
        VU() : Plugin()
        {}
        
        virtual void processRGB(uint8_t& r, uint8_t& g, uint8_t& b)
        {
            this->capture();
            this->rectify();
            
            float a = this->amplitude();
            r *= a;
            g *= a;
            b *= a;
        }
    };
}

/*
 * 
 */
void controlThread(theLEDStripControlDescription description)
{
    using boost::posix_time::milliseconds;
    using boost::posix_time::microsec_clock;

    auto transition_length = milliseconds(description.glowSpeed);
    auto frame_length = milliseconds(1000 / 25);
    
    RGBPlugin* plugin;
    if (description.filter == "music-vu")
    {
        plugin = new AudioRGBPlugins::VU;
    }
    else
    {
        plugin = new NoneRGBPlugin;
    }
    
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query("192.168.0.100", "4000");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    try
    {
        int current_transition_from = 0;
        int current_transition_to =
                (current_transition_from + 1) %
                        description.colors.size();
        auto current_transition_start = microsec_clock::local_time();
        uint16_t device_pwm_values[3] = { 0xffff, 0xffff, 0xffff };
        
        while (1)
        {
            boost::this_thread::interruption_point();            
            
            auto now = microsec_clock::local_time();
            auto next_frame_at = now + frame_length;

            auto time_since_transition_start = now - current_transition_start;
            if (time_since_transition_start > transition_length)
            {
                current_transition_from++;
                current_transition_from %= description.colors.size();
                
                current_transition_to++;
                current_transition_to %= description.colors.size();
                
                current_transition_start = microsec_clock::local_time();
                time_since_transition_start = milliseconds(0);
            }
            float transition_progress;
            if (transition_length.total_milliseconds() > 0)
            {
                transition_progress =
                    (float) time_since_transition_start.total_milliseconds() / 
                            transition_length.total_milliseconds();
            }
            else
            {
                transition_progress = 0;
            }
            uint8_t r = description.colors[current_transition_from].r
                    + (description.colors[current_transition_to].r
                        - description.colors[current_transition_from].r)
                            * transition_progress;
            uint8_t g = description.colors[current_transition_from].g
                    + (description.colors[current_transition_to].g
                        - description.colors[current_transition_from].g)
                            * transition_progress;
            uint8_t b = description.colors[current_transition_from].b
                    + (description.colors[current_transition_to].b
                        - description.colors[current_transition_from].b)
                            * transition_progress;
            
            plugin->processRGB(r, g, b);
            
            uint16_t new_pwm_values[3] = {
                // (uint16_t) (255 - r ? 4000 - 4000 * log(255 - r) / log(255) : 4000),
                // (uint16_t) (255 - g ? 4000 - 4000 * log(255 - g) / log(255) : 4000),
                // (uint16_t) (255 - b ? 4000 - 4000 * log(255 - b) / log(255) : 4000)
                (uint16_t) (r * 4000 / 255),
                (uint16_t) (g * 4000 / 255),
                (uint16_t) (b * 4000 / 255),
            };
            
            if (memcmp(device_pwm_values, new_pwm_values, 6))
            {
                memcpy(device_pwm_values, new_pwm_values, 6);
                
                boost::system::error_code ignored_error;
                boost::asio::write(socket,
                        boost::asio::buffer(device_pwm_values, 6),
                                ignored_error);
            }
            
            printf("%f\n", transition_progress);
            
            now = microsec_clock::local_time();
            if (next_frame_at > now)
            {
                auto toSleep = next_frame_at - now;
                usleep(toSleep.total_microseconds());
                // boost::this_thread::sleep is not accurate enough
            }
        }
    }
    catch (boost::thread_interrupted& e)
    {
        delete plugin;
        return;
    }
}

/*
 * 
 */
int main(int argc, char** argv)
{
    boost::thread thread;
    
    boost::asio::io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 4000));
    
    for (;;)
    {
        try
        {
            tcp::socket socket(io_service);
            acceptor.accept(socket);

            std::stringstream iss;
            boost::asio::streambuf response;
            boost::system::error_code error;
            while (boost::asio::read(socket, response,
                    boost::asio::transfer_at_least(1), error))
                iss << &response;

            if (error == boost::asio::error::eof)
            {
                theLEDStripControlDescription description;
                boost::archive::text_iarchive(iss) >> description;
                
                thread.interrupt();
                thread.join();
                
                thread = boost::thread(controlThread, description);
            }
        }
        catch (std::exception& e)
        {
        }
    }

    return 0;
}

