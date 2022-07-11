import torch

x = torch.ones(5)  # input tensor
y = torch.zeros(3)  # expected output

class Model(torch.nn.Module):
    def __init__(self):
        super(Model, self).__init__()
        self.layer = torch.nn.Linear(5, 3)

    def forward(self, x):
        return self.layer(x)

model = Model()
optimizer = torch.optim.SGD(model.parameters(), lr=0.1)
loss_fn = torch.nn.MSELoss()
for i in range(10):
    optimizer.zero_grad()
    z = model(x)
    loss = sum(y-z)
    loss.backward()
    optimizer.step()
    print(loss)
