#include <SFML/Graphics.hpp>
#include <chrono>
#include <random>
#include <iostream>

float randomFloat(const float from, const float to)
{
    unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
    static std::default_random_engine engine(seed);
    std::uniform_real_distribution<float> dist(from, to);
    return dist(engine);
}

class ParticleSystem : public sf::Drawable
{
public:
    void update(const float dt)
    {
        for (auto emitter = emitters.begin(); emitter != emitters.end();)
        {
            if (!emitter->permanent)
            {
                emitter->lifetime -= dt;
                if (emitter->lifetime <= 0.0f) emitter->alive = false;
            }

            int count = emitter->count;
            for (int i = 0; i < emitter->count; i++)
            {
                Particle& particle = emitter->particles[i];
                sf::Vertex& vertex = emitter->vertices[i];

                particle.lifetime -= dt;
                if (particle.lifetime <= 0.0f)
                {
                    vertex.position = emitter->position;
                    if (emitter->alive)
                    {
                        float parangle;
                        if (emitter->directional)
                        {
                            parangle = randomFloat(emitter->angle - emitter->deviation, emitter->angle + emitter->deviation);
                        }
                        else
                        {
                            parangle = randomFloat(0.0f, 360.0f);
                        }
                        particle.velocity.x = static_cast<float>(std::cos(parangle * 0.0174533));
                        particle.velocity.y = static_cast<float>(std::sin(parangle * 0.0174533));
                        particle.velocity *= randomFloat(1.0f, emitter->velocity);

                        particle.lifetime = randomFloat(emitter->minlifetime, emitter->maxlifetime);
                        particle.olifetime = particle.lifetime;
                        if (!emitter->explosion)
                        {
                            vertex.color = emitter->colour;
                        }
                    }
                    else
                    {
                        vertex.color = sf::Color::Transparent;
                        count--;
                    }
                }

                vertex.position += particle.velocity * dt;
                vertex.color.a = static_cast<uint8_t>((particle.lifetime / particle.olifetime) * 255);
            }

            if (!count) emitter = emitters.erase(emitter);
            else emitter++;
        }
    }

    uint16_t add(
        const int count,
        const sf::Vector2f& position,
        const sf::Color& colour,
        const float velocity,
        const float minlifetime,
        const float maxlifetime,
        const bool explosion,
        const bool permanent,
        const float lifetime,
        const bool directional,
        const float angle,
        const float deviation
    )
    {
        emitters.emplace_back(++ids, count, position, colour, velocity, minlifetime, maxlifetime, explosion, permanent, lifetime, directional, angle, deviation);
        return ids;
    }

    void remove(const uint16_t uid)
    {
        auto it = std::find(emitters.begin(), emitters.end(), uid);
        if (it == emitters.end()) return;
        it->alive = false;
    }

private:
    struct Particle
    {
        sf::Vector2f velocity;
        float olifetime;
        float lifetime;
    };

    struct Emitter
    {
        Emitter(
            const uint16_t uid,
            const int count,
            const sf::Vector2f& position,
            const sf::Color& colour,
            const float velocity,
            const float minlifetime,
            const float maxlifetime,
            const bool explosion,
            const bool permanent,
            const float lifetime,
            const bool directional,
            const float angle,
            const float deviation
        )
            : uid(uid),
            count(count),
            position(position),
            colour(colour),
            velocity(velocity),
            minlifetime(minlifetime),
            maxlifetime(maxlifetime),
            explosion(explosion),
            permanent(permanent),
            lifetime(lifetime),
            directional(directional),
            angle(angle),
            deviation(deviation),
            particles(count),
            vertices(sf::Points, count)
        {
            for (auto& particle : particles)
            {
                if (explosion)
                {
                    float parangle;
                    if (directional)
                    {
                        parangle = randomFloat(angle - deviation, angle + deviation);
                    }
                    else
                    {
                        parangle = randomFloat(0.0f, 360.0f);
                    }
                    particle.velocity.x = static_cast<float>(std::cos(parangle * 0.0174533));
                    particle.velocity.y = static_cast<float>(std::sin(parangle * 0.0174533));
                    particle.velocity *= randomFloat(1.0f, velocity);
                }
                
                particle.lifetime = randomFloat(0.0f, maxlifetime);
                particle.olifetime = particle.lifetime;
            }
            for (int i = 0; i < count; i++)
            {
                vertices[i].position = position;
                if (explosion)
                {
                    vertices[i].color = colour;
                }
                else
                {
                    vertices[i].color = sf::Color::Transparent;
                }
            }
        }
        
        bool operator==(const uint16_t rhs) { return uid == rhs; }

        uint16_t uid;
        int count;
        std::vector<Particle> particles;
        sf::VertexArray vertices;
        sf::Vector2f position;
        sf::Color colour;
        float velocity;
        float minlifetime;
        float maxlifetime;
        bool explosion;
        bool permanent;
        float lifetime;
        bool directional;
        float angle;
        float deviation;
        bool alive = true;
    };

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        sf::VertexArray vertices(sf::Points);
        for (auto& emitter : emitters)
        {
            for (unsigned int i = 0; i < emitter.vertices.getVertexCount(); i++)
            {
                vertices.append(emitter.vertices[i]);
            }
        }
        target.draw(vertices);
    }

private:
    uint16_t ids;
    std::vector<Emitter> emitters;
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(640, 480), "Particles");
    window.setFramerateLimit(60);

    sf::Clock clock;
    float dt;

    ParticleSystem particles;

    sf::Event event;
    while (window.isOpen())
    {
        dt = clock.restart().asSeconds();

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    particles.add(50, window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y)), sf::Color::Red, 50.0f, 0.5f, 2.0f, true, true, 0.0f, true, 90.0f, 20.0f);
                }
                else if (event.mouseButton.button == sf::Mouse::Right)
                {
                    particles.add(100, window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y)), sf::Color::Green, 100.0f, 0.5f, 2.0f, true, true, 0.0f, false, 0.0f, 90.0f);
                }
            }
        }

        particles.update(dt);

        window.clear();
        window.draw(particles);
        window.display();
    }

    return 0;
}