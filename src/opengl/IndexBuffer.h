class IndexBuffer {
    private:
        unsigned int m_rendererID;
        unsigned int m_length;
    public:
        IndexBuffer(const unsigned int* data, unsigned int length);
        ~IndexBuffer();

        void bind() const;
        void unbind() const;

        inline unsigned int getLength() const { return m_length; }
};